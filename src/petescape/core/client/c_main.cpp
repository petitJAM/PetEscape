
#include "petescape/core/client/client.h"
#include "petescape/core/ObjectRenderer.h"
#include "petescape/core/core_defs.h"

#include "petescape/networking/client/ClientConnection.h"
#include "petescape/networking/common/net_struct.h"

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <boost/foreach.hpp>
#include <allegro5/allegro.h>
#include <map>


namespace petescape {
namespace core {
namespace client {

#define NETWORK_RECV    513
#define NETWORK_CLOSE   555

typedef std::pair< uint32_t, GameObject* > m_element;

namespace {
//ALLEGRO_THREAD               *client_thread;
ALLEGRO_EVENT_QUEUE          *client_queue;
ALLEGRO_EVENT_SOURCE          network_event_source;
ALLEGRO_DISPLAY              *display;
ALLEGRO_TIMER                *timer;
boost::asio::io_service       client_io_service;
boost::asio::ip::tcp::socket *socket;

network_packet                input;
std::map<uint32_t, GameObject *>   objs;
std::map<uint32_t, PlayerObject *> players;

uint8_t                       client_id;

int                           map_width;
int                           map_height;
<<<<<<< HEAD
char*                         map;
=======
uint8_t                      *map;
>>>>>>> 4668f0c55ef35664624bdcd7c6b1fc66a683bfda
}

class NetworkOps_
{
public:

void async_write( packet_list list, packet_id id )
{
    if( socket->is_open() )
    {
        MESSAGE( "writing packet." );
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

void async_write_callback( const boost::system::error_code &error, size_t /*transfered*/ )
{
    if( error )
    {
        ALLEGRO_EVENT event;
        event.type = NETWORK_CLOSE;
        MESSAGE( "Error on write: " << error.message() );
        al_emit_user_event( &network_event_source, &event, nullptr );
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

        MESSAGE( "Got packet." );
    }

    al_emit_user_event( &network_event_source, &event, nullptr );

    al_unlock_mutex( input_mutex );

    async_read();
}

};

namespace {
NetworkOps_ NetOps;
}

class GameOps_
{
public:

    void updateObject( const update_obj *data )
    {
        if( objs.count( data->id ) == 1 )
        {
            objs[data->id]->setX( data->x );
            objs[data->id]->setY( data->y );
        }
    }

    void genObject( const introduce_obj *data)
    {
        if( objs.count( data->id ) )
        {
            updateObject( data );
        }
        else
        {
            GameObject *obj = nullptr;

            switch( data->type )
            {
            case PlayerType:
                obj = PlayerObject::CreatePlayer( data->id );
                players[ data->id ] = static_cast<PlayerObject*>(obj);
            break;
            case OtherType:
                obj = GameObject::CreateGameObject( data->id );
                objs[ data->id ] = obj;
            break;
            default:
                MESSAGE( "Unsupported Type. Defaulting to basic type." );
                obj = GameObject::CreateGameObject( data->id );
                objs[ data->id ] = obj;
            break;
            }

            obj->setX( data->x );
            obj->setY( data->y );
            obj->setRenderer( new petescape::core::PoorRenderer );
        }
    }

    void destroyObject( const destroy_obj *data )
    {
        objs.erase( data->id );
    }

    void handlePacket( const network_packet *packet )
    {
        packet_list new_packet;

        switch( packet->head.opcode )
        {
        case S_INFO:
            client_id = packet->data.s_info.client_id;
            NetOps.async_write( new_packet, C_READY );
            MESSAGE( "recieved S_INFO, sending C_READY" );
        break;
        case S_MAP_HEADER:
            map_width = packet->data.s_map_header.stage_length;
            map_height = packet->data.s_map_header.stage_height;
            std::cerr << "received S_MAP_HEADER, sending C_REQUEST_MAP and C_BUILD_OBJECTS" << std::endl;
            std::cerr << "expecting map of size " << map_width << " x " << map_height << std::endl;
            NetOps.async_write(new_packet, C_REQUEST_MAP);
            MESSAGE( "recieved S_MAP_HEADER, sending C_REQUEST_MAP and C_BUILD_OBJECTS" );
            NetOps.async_write(new_packet, C_BUILD_OBJECTS);
        break;
        case S_MAP_DATA:
            std::cerr << "received S_MAP_DATA" << std::endl;
            std::cerr << "i would print the map" << std::endl;

            map = (char *) packet->data2;
            std::cerr << "sizeof data: " << sizeof(map) << std::endl;
//            std::cerr << (void *) map << std::endl;

            /*
            std::cerr << "map[0]: " << (char) map[0];
            /*
            for (uint32_t i = 0; i < 12; i++)
            {
                for (uint32_t j = 0; j < 12; j++)
                {
                    std::cerr << map[i + j*12];
                }
                std::cerr << std::endl;
            }
            */
        break;
        case O_INTRODUCE:
            genObject( &packet->data.o_introduce );

            MESSAGE( "recieved O_INTRODUCE" );
        break;
        case O_UPDATE:
            updateObject( &packet->data.o_update );
            MESSAGE( "recieved O_UPDATE" );
        break;
        case O_DESTORY:
            destroyObject( &packet->data.o_destroy );
            MESSAGE( "recieved O_DESTROY" );
        break;

        default:
            // Do nothing.
            MESSAGE( "Do nothing." );
        break;
        }
    }

};

namespace {
GameOps_   GameOps;
}

int c_main( int argc, char **argv )
{
    boost::asio::ip::tcp::resolver tcp_resolver( client_io_service );
    boost::asio::ip::tcp::resolver::query tcp_query( argv[2], "2001" );
    boost::asio::ip::tcp::resolver::iterator tcp_endpoint;

    tcp_endpoint = tcp_resolver.resolve( tcp_query );

    boost::asio::io_service::work work( client_io_service );
    boost::thread io_thread( boost::bind( &boost::asio::io_service::run, &client_io_service ) );

    client_queue = nullptr;
    display = nullptr;
    timer = nullptr;

    try
    {
        bool should_exit = false;
        bool redraw      = false;

        if( !al_init() )
        {
            MESSAGE( "Failed to initialize Allegro." );
            return -1;
        }

        if( !al_install_mouse() )
        {
            MESSAGE( "Error initializing Allegro mouse driver." );
            return -2;
        }

        if( !( client_queue = al_create_event_queue() ) )
        {
            MESSAGE( "Unable to create Allegro Event Queue." );
            return -3;
        }

        if( !( timer = al_create_timer( 1.0 / 30.0 ) ) )
        {
            MESSAGE( "Error initializing Allegro timer." );
            return -4;
        }

        if( !( display = al_create_display( 800, 600 ) ) )
        {
            MESSAGE( "Error initializing Allegro dispay driver." );
            al_destroy_timer( timer );
            return -5;
        }

        al_register_event_source( client_queue, al_get_display_event_source( display ) );
        al_register_event_source( client_queue, al_get_timer_event_source( timer ) );
        al_register_event_source( client_queue, al_get_mouse_event_source() );

        al_init_user_event_source( &network_event_source );
        al_register_event_source( client_queue, &network_event_source );

        al_start_timer( timer );

        // Allegro Event loop.
        while( !should_exit )
        {
            ALLEGRO_EVENT event;
            al_wait_for_event( client_queue, &event );

            switch( event.type )
            {
            case ALLEGRO_EVENT_TIMER:
                redraw = true;
            break;

            case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN:
                MESSAGE( "recieving event ALLEGRO_EVENT_MOUSE_BUTTON_DOWN" );
                if( socket == nullptr)
                {
                    packet_list padding;

                    socket = new boost::asio::ip::tcp::socket( client_io_service );
                    boost::asio::connect( *socket, tcp_endpoint );
                    MESSAGE( "Connected." );

                    NetOps.async_write( padding, C_HELLO );
                    MESSAGE( "sending C_HELLO" );

                    NetOps.async_read();
                }
            break;

            case NETWORK_RECV:
                MESSAGE( "recieving event NETWORK_RECV" );

                // Pass the packet off to the packet handler.
                GameOps.handlePacket( (network_packet *)event.user.data1 );

                // Clean up memory usage.
                delete (network_packet *)event.user.data1;
            break;

            case NETWORK_CLOSE:
                MESSAGE( "recieving event NETWORK_CLOSE" );
            break;
            case ALLEGRO_EVENT_DISPLAY_CLOSE:
                MESSAGE( "recieving event ALLEGRO_EVENT_DISPLAY_CLOSE" );
                should_exit = true;
            break;
            }

            if( redraw && al_is_event_queue_empty( client_queue ) )
            {

                // Set screen to white.
                al_clear_to_color( al_map_rgb( 255, 255, 255 ) );

                // Rendering code goes here
                BOOST_FOREACH( m_element tmp, objs )
                {
                    ((GameObject*)(tmp.second))->render();
                }


                BOOST_FOREACH( m_element tmp, players )
                {
                    ((GameObject*)(tmp.second))->render();
                }

                al_flip_display();
                redraw = false;
            }
        }

        client_io_service.stop();

        io_thread.join();

        if( timer )        al_destroy_timer( timer );
        if( display )      al_destroy_display( display );
        if( client_queue ) al_destroy_event_queue( client_queue );
    }
    catch( std::exception &e )
    {
        MESSAGE( "Error: " << e.what() );
        return -1;
    }

    return 0;
}

}}}
