
#include "petescape/core/server/server.h"
#include "petescape/core/core_defs.h"
#include "petescape/networking/common/net_struct.h"
#include "petescape/core/GameMap.h"
//#include "petescape/networking/server/ServerConnection.h"

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/foreach.hpp>
#include <boost/thread/thread.hpp>
#include <iterator>
#include <allegro5/allegro.h>

namespace petescape {
namespace core {
namespace server {

#define NETWORK_CONNECT 512
#define NETWORK_RECV    513
#define NETWORK_CLOSE   555

namespace {
ALLEGRO_EVENT_QUEUE          *server_queue;
ALLEGRO_EVENT_SOURCE          network_event_source;
boost::asio::io_service       server_io_service;
boost::asio::ip::tcp::socket *socket;

network_packet                input;

GameMap                       map;

std::map<uint32_t, GameObject *>   objs;
std::map<uint32_t, PlayerObject *> players;

uint8_t                       map_length;
uint8_t                       map_height;
}

class NetworkOps_
{
public:

void async_write( packet_list list, packet_id id )
{
    if( socket->is_open() )
    {
        network_packet packet;
        packet.data = list;
        packet.head.opcode = id;
        packet.head.response_required = 0;
        packet.head.sender_id = 0;

        boost::asio::async_write( *socket,
                                  boost::asio::buffer( &packet, sizeof( packet ) ),
                                  boost::bind( &NetworkOps_::async_write_callback,
                                               this,
                                               boost::asio::placeholders::error,
                                               boost::asio::placeholders::bytes_transferred ) );
    }
    else
    {
        MESSAGE( "Error: socket unavailable to write to." );
    }
}

void transfer_map(GameMap map)
{
    if( socket->is_open() )
    {
        MESSAGE( "writing packet." );
        MESSAGE( map.getSize() );
        unsigned int packet_number = 0;

        while(packet_number * MAP_PACKET_SIZE < map.getSize()){
            packet_list new_packet;
            new_packet.s_map_data.packet_number = packet_number;

            map.populateChunk(new_packet.s_map_data);

            async_write(new_packet, S_MAP_DATA);
            MESSAGE( "writing S_MAP_DATA " << (packet_number + 1));
            packet_number++;
        }
        /*
        while(packet_number < (map.getSize() / MAP_PACKET_SIZE)){
            packet_list new_packet;
            new_packet.s_map_data.packet_number = packet_number;

            map.populateChunk(new_packet.s_map_data);

            async_write(new_packet, S_MAP_DATA);
            MESSAGE( "writing S_MAP_DATA " << (packet_number + 1));
            packet_number++;
        }

        //Might need to do one more smaller packet
        if(packet_number * MAP_PACKET_SIZE < map.getSize()){
            packet_list new_packet;
            new_packet.s_map_data.packet_number = packet_number;
            unsigned int current_packet = 0;
            while(packet_number * MAP_PACKET_SIZE + current_packet < size){
                new_packet.s_map_data.data_group[current_packet] = data[packet_number * MAP_PACKET_SIZE + current_packet];
            }
            async_write(new_packet, S_MAP_DATA);
            MESSAGE( "writing S_MAP_DATA " << (packet_number + 1));
        }
        */
    }
    else
    {
        MESSAGE( "Error: socket unavailable to write to." );
    }
}

void async_write_callback( const boost::system::error_code &error, size_t /*transfered*/ )
{
    if( error )
    {
        MESSAGE( "Error on write: " << error.message() );
    }
}

void async_read()
{
    if( socket->is_open() )
    {
        boost::asio::async_read( *socket,
                                 boost::asio::buffer( &input, sizeof( input ) ),
                                 boost::bind( &NetworkOps_::async_read_callback,
                                              this,
                                              boost::asio::placeholders::error,
                                              boost::asio::placeholders::bytes_transferred ) );
    }
    else
    {
        MESSAGE( "Error: socket unavailable to read from." );
    }
}

void async_read_callback( const boost::system::error_code &error, size_t /*transfered*/ )
{
    static ALLEGRO_MUTEX *input_mutex = al_create_mutex();

    al_lock_mutex( input_mutex );
    ALLEGRO_EVENT event;

    if( error )
    {
        event.type = NETWORK_CLOSE;
        MESSAGE( "Error on read: " << error.message() );
    }
    else
    {
        network_packet *packet;

        packet = new network_packet;
        packet->data = input.data;
        packet->head = input.head;

        event.type = NETWORK_RECV;
        event.user.data1 = (intptr_t)packet;
    }

    al_emit_user_event( &network_event_source, &event, nullptr );

    al_unlock_mutex( input_mutex );

    async_read();
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
            NetworkOps.async_write( new_packet, S_INFO );
            MESSAGE( "recieved C_HELLO, write with S_INFO" );
        }break;

        case C_READY: {
            // Send the client anything that it needs. In this case, a map header.
            map_length = MAP_LENGTH;
            map_height = MAP_HEIGHT;
            new_packet.s_map_header.stage_length = map_length;
            new_packet.s_map_header.stage_height = map_height;

            NetworkOps.async_write(new_packet, S_MAP_HEADER);
            MESSAGE( "recieved C_READY, write with S_MAP_HEADER" );
        } break;

        case C_REQUEST_MAP: {
            //Begin sending the client a stream of map information.
            map = GameMap::GameMap(map_height, map_length);
            // Init Map Data
            map.generate();

            // just to look at the map - broken at the moment.
            /*
            for (uint32_t i = 0; i < MAP_HEIGHT; i++)
            {
                for (uint32_t j = 0; j < MAP_LENGTH; j++)
                    printf("%d", map.getValue(j, i));
                printf("\n");
            }
            */
            NetworkOps.transfer_map(map);

            MESSAGE( "recieved C_REQUEST_MAP" );
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

                NetworkOps.async_write( new_packet, O_INTRODUCE );
                ++count;
            }

            BOOST_FOREACH( map_element i, objs )
            {
                new_packet.o_introduce.id   = ((GameObject*)(i.second))->getID();
                new_packet.o_introduce.type = (uint16_t)OtherType;
                new_packet.o_introduce.x    = (uint32_t)((GameObject*)(i.second))->getX();
                new_packet.o_introduce.y    = (uint32_t)((GameObject*)(i.second))->getY();

                NetworkOps.async_write( new_packet, O_INTRODUCE );
                ++count;
            }
            MESSAGE( "Wrote " << count << " objects." );
            MESSAGE( "recieved C_BUILD_OBJECTS, write with O_INTRODUCE" );
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

    uint8_t* generateMapData()
    {
        uint32_t length = MAP_LENGTH * MAP_HEIGHT;
        uint8_t* map = new uint8_t[length];

        // gens map like this (with different size):
        // 00000
        // 00000
        // 11111
        for (uint32_t i = 0; i < length; i++)
        {
            if (i % MAP_HEIGHT == MAP_HEIGHT - 1)
                map[i] = 1;
            else
                map[i] = 0;
        }

        // seed rand
        srand(123456);
        // populate with random platforms
        int n_plats = rand() % 50, plat_len, plat_x, plat_y;

        if (MAP_LENGTH > 10)
        {
            for (int i = 0; i<n_plats; i++)
            {
                plat_len = rand() % 5;
                plat_x = (rand() % (MAP_LENGTH - 10)) + 5; // how far over
                plat_y = (rand() % (MAP_HEIGHT - 10)) + 2; // how far up

                for (int j = 0; j<plat_len; j++)
                {
                    map[plat_y + ((plat_x + j - 1) * MAP_HEIGHT)] = 1;
                }
            }
        }

        return map;
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

        NetworkOps.async_read();
    }
}

int s_main( int /*argc*/, char ** /*argv*/ )
{
    boost::asio::ip::tcp::acceptor tcp_acceptor( server_io_service,
                                                 boost::asio::ip::tcp::endpoint(
                                                     boost::asio::ip::tcp::v4(),
                                                     2001 ) );

    boost::asio::io_service::work work( server_io_service );
    boost::thread io_thread( boost::bind( &boost::asio::io_service::run, &server_io_service ) );

    server_queue  = nullptr;

    try
    {
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

        al_init_user_event_source( &network_event_source );
        al_register_event_source( server_queue, &network_event_source );

        socket = new boost::asio::ip::tcp::socket( server_io_service );

        // Setup async_accept.
        tcp_acceptor.async_accept( *socket,
                                   boost::bind( &accept_handler,
                                                boost::asio::placeholders::error ) );

        // allegro event queue
        while( !should_exit )
        {
            ALLEGRO_EVENT event;
//            uint32_t tmp;
            al_wait_for_event( server_queue, &event );

            switch( event.type )
            {
            case NETWORK_CONNECT:
                MESSAGE( "recieving event NETWORK_CONNECT" );
                // If we need to do anything when the Client connects, it goes here.
                // For testing, we create a GameObject, and move it to 100,100.
//                tmp = ServerOps.genObject();
//                objs[ tmp ]->setX( 100 );
//                objs[ tmp ]->setY( 100 );


            break;

            case NETWORK_RECV:
                MESSAGE( "recieving event NETWORK_RECV" );

                // Do something based off what kind of packet it is.
                ServerOps.handlePacket( (network_packet *)event.user.data1 );

                // Dealloc the memory we used for the packet.
                delete (network_packet *)event.user.data1;
            break;

            case NETWORK_CLOSE:
                MESSAGE( "recieving event NETWORK_CLOSE" );
                should_exit = true;
            break;

            default:
                MESSAGE( "Ignored Event." );
            }

            // TODO timer
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
