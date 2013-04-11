
#include "petescape/core/server/server.h"
#include "petescape/networking/common/net_struct.h"
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
std::map<uint32_t, GameObject *> objs;

char*                         map;
uint32_t                      map_height = 12;
uint32_t                      map_width = 12;
}

class NetworkOps_
{
public:

void async_write( packet_list list, packet_id id )
{
    if( socket->is_open() )
    {
        std::cerr << "writing packet." << std::endl;
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
        std::cerr << "Error: socket unavailable to write to." << std::endl;
    }
}

void async_write(void* data)
{
    if( socket->is_open() )
    {
        std::cerr << "writing packet." << std::endl;
        std::cerr << sizeof(char) << std::endl;
        std::cerr << "sizeof data sent is " << sizeof(data) << std::endl;

        // this won't work in the end
        network_packet packet;
        //packet.data = s_map_data;
        packet.data2 = data;
        packet.head.opcode = S_MAP_DATA;
        packet.head.response_required = 0;
        packet.head.sender_id = 0;

        // xTODOx this code doesn't work becuse the sizeof throws an illegal indirection when written properly
        // that's no longer the problem. it seems that this packet is sending the pointer and not the data itself.
        boost::asio::async_write( *socket,
                                  boost::asio::buffer( &packet, sizeof( packet ) ),
                                  boost::bind( &NetworkOps_::async_write_callback,
                                               this,
                                               boost::asio::placeholders::error,
                                               boost::asio::placeholders::bytes_transferred ) );
    }
    else
    {
        std::cerr << "Error: socket unavailable to write to." << std::endl;
    }
}

void async_write_callback( const boost::system::error_code &error, size_t /*transfered*/ )
{
    if( error )
    {
        std::cerr << "Error on write: " << error.message() << std::endl;
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
        std::cerr << "Error: socket unavailable to read from." << std::endl;
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
        std::cerr << "Error on read: " << error.message() << std::endl;
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
        static uint8_t NextID = 0;
        packet_list new_packet;
        // TODO put this in a better spot so map is saved on the server
        char* map;
        char* a;

        switch( packet->head.opcode )
        {
        case C_HELLO:
            new_packet.s_info.client_id = NextID++;

            // Tell the client its ID
            NetworkOps.async_write( new_packet, S_INFO );
            std::cerr << "recieved C_HELLO, write with S_INFO" <<std::endl;
        break;

        case C_READY:
            // Send the client anything that it needs. In this case, a map header.
            new_packet.s_map_header.stage_length = map_width;
            new_packet.s_map_header.stage_height = map_height;

            NetworkOps.async_write(new_packet, S_MAP_HEADER);
            std::cerr << "recieved C_READY, write with S_MAP_HEADER" << std::endl;
        break;

        case C_REQUEST_MAP:
            //Begin sending the client a stream of map information.

            // Init Map Data
            map = generateMapData(map_height, map_width);

            // just to look at the map
            for (uint32_t i = 0; i < 12; i++)
            {
                for (uint32_t j = 0; j < 12; j++)
                    printf("%d  ", map[i + j*12]);
                printf("\n");
            }

            // would use this
            // NetworkOps.async_write(map);

            // using simpler instead
            a = new char[3];
            a[0] = 'a'; a[1] = 'a'; a[2] = 'a';
            NetworkOps.async_write(a);

            std::cerr << "recieved C_REQUEST_MAP, method needs to be worked on." << std::endl;
        break;

        case C_BUILD_OBJECTS:
            BOOST_FOREACH( map_element i, objs )
            {
                new_packet.o_introduce.id = ((GameObject*)(i.second))->getID();
                new_packet.o_introduce.x  = (uint32_t)((GameObject*)(i.second))->getX();
                new_packet.o_introduce.y  = (uint32_t)((GameObject*)(i.second))->getY();

                NetworkOps.async_write( new_packet, O_INTRODUCE );
            }
            std::cerr << "Wrote " <<( new_packet.o_introduce.id+1) << " objects." << std::endl;
            std::cerr << "recieved C_BUILD_OBJECTS, write with O_INTRODUCE" << std::endl;
        break;

        default:
            // Do nothing.
            std::cerr << "Do nothing." << std::endl;
        break;
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

    char* generateMapData(uint32_t height, uint32_t width)
    {
        uint32_t length = height * width;
        char* map = new char[length];

        for (uint32_t i = 0; i < length; i++)
        {
            if (i % height == height - 1)
                map[i] = 1;
            else
                map[i] = 0;
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
        std::cerr << "Error: " << error.message() << std::endl;
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
            std::cerr << "Failed to initialize Allegro.\n";
            return -1;
        }

        if( !( server_queue = al_create_event_queue() ) )
        {
            std::cerr << "Unable to create Allegro Event Queue.\n";
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
            uint32_t tmp;
            al_wait_for_event( server_queue, &event );

            switch( event.type )
            {
            case NETWORK_CONNECT:
                std::cerr << "recieving event NETWORK_CONNECT" << std::endl;
                // If we need to do anything when the Client connects, it goes here.
                // For testing, we create a GameObject, and move it to 100,100.
                tmp = ServerOps.genObject();
                objs[ tmp ]->setX( 100 );
                objs[ tmp ]->setY( 100 );


            break;

            case NETWORK_RECV:
                std::cerr << "recieving event NETWORK_RECV" << std::endl;

                // Do something based off what kind of packet it is.
                ServerOps.handlePacket( (network_packet *)event.user.data1 );

                // Dealloc the memory we used for the packet.
                delete (network_packet *)event.user.data1;
            break;

            case NETWORK_CLOSE:
                std::cerr << "recieving event NETWORK_CLOSE" << std::endl;
                should_exit = true;
            break;

            default:
                std::cerr << "Ignored Event." << std::endl;
            }

            // TODO timer
        }

        server_io_service.stop();

        io_thread.join();

        if( server_queue ) al_destroy_event_queue( server_queue );
    }
    catch( std::exception &e )
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    return 0;
}

}}}
