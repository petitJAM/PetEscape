
#include "petescape/core/server/server.h"
#include "petescape/core/core_defs.h"
#include "petescape/networking/common/net_struct.h"
#include "petescape/core/GameMap.h"

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/foreach.hpp>
#include <boost/thread/thread.hpp>
#include <map>
#include <iterator>
#include <allegro5/allegro.h>

namespace petescape {
namespace core {
namespace server {

#define NETWORK_CONNECT 512
#define NETWORK_RECV    513
#define NETWORK_CLOSE   555

namespace {
ALLEGRO_EVENT_QUEUE     *server_queue;
ALLEGRO_EVENT_SOURCE     network_event_source;
boost::asio::io_service  server_io_service;

boost::asio::ip::tcp::acceptor *tcp_acceptor;
boost::asio::ip::tcp::socket   *waiting_socket;
boost::asio::ip::tcp::socket   *sockets[ MAX_CONNECTIONS ];
uint8_t                         connection_count;

network_packet  waiting_packets[ MAX_CONNECTIONS ];
bool            expecting_error_on_client[ MAX_CONNECTIONS ];

GameMap *map;
uint8_t  map_length;
uint8_t  map_height;

std::map<uint32_t, GameObject *>   objs;
std::map<uint32_t, PlayerObject *> players;

time_t pressed_keys[MAX_CONNECTIONS][TOTAL_SUPPORTED_KEYS];
bool players_ducking[MAX_CONNECTIONS];
}

class NetworkOps_
{
public:


    void async_write( uint8_t client_id, packet_list &list, packet_id id )
    {
        if( sockets[ client_id ]->is_open() )
        {
            network_packet packet;
            packet.data = list;
            packet.head.opcode = id;
            packet.head.response_required = 0;
            packet.head.sender_id = 0;

            boost::asio::async_write( *( sockets[ client_id ] ),
                                      boost::asio::buffer( &packet, sizeof( packet ) ),
                                      boost::bind( &NetworkOps_::async_write_callback,
                                                   this,
                                                   client_id,
                                                   boost::asio::placeholders::error,
                                                   boost::asio::placeholders::bytes_transferred ) );
        }
        else
        {
            MESSAGE( "Error: socket unavailable to write to." );
        }
    }

    void transfer_map( uint8_t client_id, GameMap *map )
    {
        if( sockets[ client_id ]->is_open() )
        {
            MESSAGE( "writing packet." );
            MESSAGE( map->getSize() );
            unsigned int packet_number = 0;

            for( int i = 0; i < map->getSize(); i+=MAP_PACKET_SIZE )
            {
    //        while(packet_number * MAP_PACKET_SIZE < map->getSize()){
                packet_list new_packet;
                new_packet.s_map_data.packet_number = i/MAP_PACKET_SIZE;

                map->populateChunk(new_packet.s_map_data);

                async_write(client_id, new_packet, S_MAP_DATA);
                MESSAGE( "writing S_MAP_DATA " << (packet_number + 1));
                packet_number++;
            }
        }
        else
        {
            MESSAGE( "Error: socket unavailable to write to." );
        }
    }

    void async_write_callback( uint8_t client_id, const boost::system::error_code &error, size_t /*transfered*/ )
    {
        if( error )
        {
            MESSAGE( "Error on write to client" << (uint32_t)client_id << "\n\t" << error.message() );
        }
    }

    void async_read( uint8_t client_id )
    {
        if( sockets[ client_id ]->is_open() )
        {
            boost::asio::async_read( *( sockets[ client_id ] ),
                                     boost::asio::buffer( &( waiting_packets[ client_id ] ),
                                                          sizeof( waiting_packets[ client_id ] ) ),
                                     boost::bind( &NetworkOps_::async_read_callback,
                                                  this,
                                                  client_id,
                                                  boost::asio::placeholders::error,
                                                  boost::asio::placeholders::bytes_transferred ) );
        }
        else
        {
            MESSAGE( "Error: socket unavailable to read from." );
        }
    }

    void async_read_callback( uint8_t client_id, const boost::system::error_code &error, size_t /*transfered*/ )
    {
        // TODO: This is hella ghetto. Needs not to be.
        static ALLEGRO_MUTEX* input_mutex[ MAX_CONNECTIONS ] =
        { al_create_mutex(), al_create_mutex(), al_create_mutex(), al_create_mutex() };

        al_lock_mutex( input_mutex[ client_id ] );
        ALLEGRO_EVENT event;

        if( error && !expecting_error_on_client[ client_id ] )
        {
            MESSAGE( "Error on read: " << error.message() << "\n\tAttempting a graceful socket close." );

            event.type = NETWORK_CLOSE;
            event.user.data1 = client_id;
            al_unlock_mutex( input_mutex[ client_id ] );
            al_emit_user_event( &network_event_source, &event, nullptr );
            return;
        }
        if( expecting_error_on_client[ client_id ] )
        {
            MESSAGE( "Ending read sequence for client " << (uint32_t)client_id << " greacefuly." );

            expecting_error_on_client[ client_id ] = false;
            al_unlock_mutex( input_mutex[ client_id ] );
            return;
        }
        else
        {
            network_packet *packet;

            packet = new network_packet;
            packet->data = waiting_packets[ client_id ].data;
            packet->head = waiting_packets[ client_id ].head;

            event.type = NETWORK_RECV;
            event.user.data1 = (intptr_t)packet;
        }

        al_emit_user_event( &network_event_source, &event, nullptr );

        al_unlock_mutex( input_mutex[ client_id ] );

        async_read( client_id );
    }

};

namespace {
NetworkOps_ NetworkOps;
}

class ServerOps_
{
    typedef std::pair<uint32_t, GameObject*> map_element;

public:

    void handlePacket( const network_packet *packet )
    {
        packet_list new_packet;
        // TODO put this in a better spot so map is saved on the server

        switch( packet->head.opcode )
        {
        case C_HELLO: {
            PlayerObject *player = PlayerObject::CreatePlayer();
            players[ player->getID() ] = player;

            new_packet.s_info.client_id = player->getID();

            // Tell the client its ID
            NetworkOps.async_write( player->getID(), new_packet, S_INFO );
            MESSAGE( "recieved C_HELLO, write with S_INFO" );

            // Tell the other connections about the new player.
            for( int i = 0; i < MAX_CONNECTIONS; ++i )
            {
                if( ( i != player->getID() ) && ( sockets[ i ] != nullptr ) )
                {
                    packet_list new_packet;
                    new_packet.o_introduce.id   = player->getID();
                    new_packet.o_introduce.type = (uint16_t)PlayerType;
                    new_packet.o_introduce.x    = (uint32_t)player->getX();
                    new_packet.o_introduce.y    = (uint32_t)player->getY();

                    NetworkOps.async_write( i, new_packet, O_INTRODUCE );
                }
            }
        }break;

        case C_CLOSE: {
            MESSAGE( "Client " << (uint32_t)packet->head.sender_id << " is closing." );
            ALLEGRO_EVENT event;

            expecting_error_on_client[ packet->head.sender_id ] = true;

            event.type = NETWORK_CLOSE;
            event.user.data1 = packet->head.sender_id;
            al_emit_user_event( &network_event_source, &event, nullptr );
        } break;

        case C_READY: {
            // Send the client anything that it needs. In this case, a map header.
            map_length = MAP_LENGTH;
            map_height = MAP_HEIGHT;
            new_packet.s_map_header.stage_length = map_length;
            new_packet.s_map_header.stage_height = map_height;

            NetworkOps.async_write( packet->head.sender_id, new_packet, S_MAP_HEADER);

            MESSAGE( "recieved C_READY, write with S_MAP_HEADER" );
        } break;

        case C_REQUEST_MAP: {
            //Begin sending the client a stream of map information.
            if( map == nullptr )
            {
                map = new GameMap(map_height, map_length);

                // Init Map Data
                map->generate();

                // take a peek
                //map->display();
            }

            NetworkOps.transfer_map( packet->head.sender_id, map );
            MESSAGE( "recieved C_REQUEST_MAP" );

            // tell them that's all they need.
            NetworkOps.async_write( packet->head.sender_id, new_packet, S_READY );

        } break;

        case C_BUILD_OBJECTS: {
            int count = 0;
            // Write Each player
            BOOST_FOREACH( map_element i, players )
            {
                new_packet.o_introduce.id   = ((PlayerObject*)(i.second))->getID();
                new_packet.o_introduce.type = (uint16_t)PlayerType;
                new_packet.o_introduce.x    = (uint32_t)((PlayerObject*)(i.second))->getX();
                new_packet.o_introduce.y    = (uint32_t)((PlayerObject*)(i.second))->getY();

                NetworkOps.async_write( packet->head.sender_id, new_packet, O_INTRODUCE );
                ++count;
            }

            BOOST_FOREACH( map_element i, objs )
            {
                new_packet.o_introduce.id   = ((GameObject*)(i.second))->getID();
                new_packet.o_introduce.type = (uint16_t)OtherType;
                new_packet.o_introduce.x    = (uint32_t)((GameObject*)(i.second))->getX();
                new_packet.o_introduce.y    = (uint32_t)((GameObject*)(i.second))->getY();

                NetworkOps.async_write( packet->head.sender_id, new_packet, O_INTRODUCE );
                ++count;
            }
            MESSAGE( "Wrote " << count << " objects." );
            MESSAGE( "recieved C_BUILD_OBJECTS, write with O_INTRODUCE" );
        } break;

        case O_UPDATE: {

            switch( packet->data.o_update.type )
            {
            case PlayerType:
                players[ packet->head.sender_id ]->setX( packet->data.o_update.x );
                players[ packet->head.sender_id ]->setY( packet->data.o_update.y );
                players[ packet->head.sender_id ]->set_facing( packet->data.o_update.facing );

                BOOST_FOREACH( map_element i, players )
                {
                    if( ((PlayerObject*)(i.second))->getID() == packet->head.sender_id )
                        continue;

                    new_packet.o_update.id   = packet->data.o_update.id;
                    new_packet.o_update.type = (uint16_t)PlayerType;
                    new_packet.o_update.x    = packet->data.o_update.x;
                    new_packet.o_update.y    = packet->data.o_update.y;
                    new_packet.o_update.facing = packet->data.o_update.facing;

                    NetworkOps.async_write( ((PlayerObject*)(i.second))->getID(), new_packet, O_UPDATE );
                }

                break;
            }

        } break;

        case C_USER_INPUT: {
            //time of the event
            time_t event_time = packet->data.c_user_input.event_time;
            uint8_t client_id = packet->head.sender_id;

            switch( packet->data.c_user_input.event.type )
            {
            case ALLEGRO_EVENT_KEY_DOWN: {
                //pull out the event
                ALLEGRO_KEYBOARD_EVENT event = packet->data.c_user_input.event.keyboard;

                switch( event.keycode )
                {
                case ALLEGRO_KEY_W: {
                    pressed_keys[client_id][KEY_UP_INDEX] = event_time;
                } break;
                case ALLEGRO_KEY_A: {
                    pressed_keys[client_id][KEY_LEFT_INDEX] = event_time;
                } break;
                case ALLEGRO_KEY_S: {
                    pressed_keys[client_id][KEY_DOWN_INDEX] = event_time;
                    players_ducking[client_id] = true;
                } break;
                case ALLEGRO_KEY_D: {
                    pressed_keys[client_id][KEY_RIGHT_INDEX] = event_time;
                } break;
                case ALLEGRO_KEY_SPACE: {
                    pressed_keys[client_id][KEY_SPACE_INDEX] = event_time;
                } break;
                }
            } break;
            case ALLEGRO_EVENT_KEY_UP: {
                //pull out the event
                ALLEGRO_KEYBOARD_EVENT event = packet->data.c_user_input.event.keyboard;

                switch( event.keycode )
                {
                case ALLEGRO_KEY_W: {
                    time_t time_pressed = packet->data.c_user_input.event_time - pressed_keys[client_id][KEY_UP_INDEX];
                    pressed_keys[client_id][KEY_UP_INDEX] = (time_t) 0;
                } break;
                case ALLEGRO_KEY_A: {
                    time_t time_pressed = packet->data.c_user_input.event_time - pressed_keys[client_id][KEY_LEFT_INDEX];
                    //some type of position updating using the time pressed and the player's speed.
                    pressed_keys[client_id][KEY_LEFT_INDEX] = (time_t) 0;
                } break;
                case ALLEGRO_KEY_S: {
                    //time_t time_pressed = packet->data.c_user_input.event_time - pressed_keys[client_id][KEY_DOWN_INDEX];
                    players_ducking[client_id] = false;
                    pressed_keys[client_id][KEY_DOWN_INDEX] = (time_t) 0;
                } break;
                case ALLEGRO_KEY_D: {
                    time_t time_pressed = packet->data.c_user_input.event_time - pressed_keys[client_id][KEY_RIGHT_INDEX];
                    pressed_keys[client_id][KEY_RIGHT_INDEX] = (time_t) 0;
                } break;
                case ALLEGRO_KEY_SPACE: {
                    time_t time_pressed = packet->data.c_user_input.event_time - pressed_keys[client_id][KEY_SPACE_INDEX];
                    pressed_keys[client_id][KEY_SPACE_INDEX] = (time_t) 0;
                } break;
                }
            } break;
            default: {
                MESSAGE("Not written");
            }break;
            }
        } break;

        default: {
            // Do nothing.
            MESSAGE( "Do nothing." );
        } break;
        }
    }

    uint32_t genObject()
    {
        GameObject *obj = GameObject::CreateGameObject( );
        objs[ obj->getID() ] = obj;
        return obj->getID();
    }


    void destroyObject( GameObject *obj )
    {
        objs.erase( obj->getID() );
    }
};

namespace {
ServerOps_ ServerOps;
}


static void accept_handler( const boost::system::error_code &error )
{
    if( error )
    {
        MESSAGE( "Error: " << error.message() );
    }
    else
    {
        ALLEGRO_EVENT event;
        event.type = NETWORK_CONNECT;
        al_emit_user_event( &network_event_source, &event, nullptr );

        sockets[ connection_count ] = waiting_socket;

        NetworkOps.async_read( connection_count );
        ++connection_count;

        if( connection_count != MAX_CONNECTIONS )
        {
            waiting_socket = new boost::asio::ip::tcp::socket( server_io_service );

            tcp_acceptor->async_accept( *waiting_socket,
                                        boost::bind( &accept_handler,
                                                     boost::asio::placeholders::error ) );
        }

    }
}

int s_main( int /*argc*/, char ** /*argv*/ )
{
    tcp_acceptor = new boost::asio::ip::tcp::acceptor( server_io_service,
                                                       boost::asio::ip::tcp::endpoint(
                                                           boost::asio::ip::tcp::v4(),
                                                           2001 ) );

    boost::asio::io_service::work work( server_io_service );
    boost::thread io_thread( boost::bind( &boost::asio::io_service::run, &server_io_service ) );
    uint64_t loop_counter = 0;
    ALLEGRO_EVENT event;

    server_queue  = nullptr;
    map = nullptr;
    connection_count = 0;

    for( int i = 0;
         i < MAX_CONNECTIONS;
         expecting_error_on_client[ i ] = false,
         sockets[ i ] = nullptr,
         ++i );

    try
    {
        ALLEGRO_TIMER *logic_timer;
        bool should_exit = false;

        if( !al_init() )
        {
            MESSAGE( "Failed to initialize Allegro." );
            return -1;
        }

        if( !( server_queue = al_create_event_queue() ) )
        {
            MESSAGE( "Unable to create Allegro Event Queue." );
            return -2;
        }

        if( !( logic_timer = al_create_timer( 1.0 / 30.0 ) ) )
        {
            MESSAGE( "Unable to create Allegro Timer." );
            return -3;
        }

        al_init_user_event_source( &network_event_source );
        al_register_event_source( server_queue, &network_event_source );
        al_register_event_source( server_queue, al_get_timer_event_source( logic_timer ) );

        waiting_socket = new boost::asio::ip::tcp::socket( server_io_service );

        // Setup async_accept.
        tcp_acceptor->async_accept( *waiting_socket,
                                    boost::bind( &accept_handler,
                                                 boost::asio::placeholders::error ) );

        al_start_timer( logic_timer );

        // allegro event queue
        while( !should_exit )
        {
            al_wait_for_event( server_queue, &event );

            switch( event.type )
            {

            case ALLEGRO_EVENT_TIMER: {
                static uint8_t update_id = 0;
                // update objects;

                // Calculate player to update:
                // id = loop_counter % MAX_CONNECTIONS
                update_id = loop_counter % MAX_CONNECTIONS;
                if( sockets[ update_id ] )
                {
                    // push obj info to client with ID = update_id
                }
                ++loop_counter;

            } break;

            case NETWORK_CONNECT:
                MESSAGE( "recieving event NETWORK_CONNECT" );
            break;

            case NETWORK_RECV:
                MESSAGE( "recieving event NETWORK_RECV" );

                // Do something based off what kind of packet it is.
                ServerOps.handlePacket( (network_packet *)event.user.data1 );

                // Dealloc the memory we used for the packet.
                delete (network_packet *)event.user.data1;
            break;

            case NETWORK_CLOSE: {
                MESSAGE( "recieving event NETWORK_CLOSE" );

                should_exit = true;

                sockets[ event.user.data1 ]->close();
                delete sockets[ event.user.data1 ];
                sockets[ event.user.data1 ] = nullptr;

                for( int i = 0; i < MAX_CONNECTIONS; ++i )
                {
                    if( sockets[ i ] != nullptr )
                        should_exit = false;
                }

            } break;

            default:
                MESSAGE( "Ignored Event." );
            }
        }

        server_io_service.stop();

        io_thread.join();

        if( server_queue ) al_destroy_event_queue( server_queue );
    }
    catch( std::exception &e )
    {
        MESSAGE( e.what() );
        return -1;
    }

    return 0;
}

}}}
