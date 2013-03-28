
#include "petescape/core/client/client.h"
#include "petescape/networking/client/ClientConnection.h"
#include "petescape/networking/common/net_struct.h"

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <allegro5/allegro.h>


namespace petescape {
namespace core {
namespace client {

using petescape::networking::client::ClientConnection;

PetEscapeClient::PetEscapeClient(boost::asio::io_service &io, const char* ipAddress) :
    m_resolver( io )
{
#ifdef DEBUG
    std::cout << "Here?\n";
    std::cout << ipAddress << "\n";
#endif
    tcp::resolver::query q( ipAddress, "2001" );
    tcp::resolver::iterator end_point = m_resolver.resolve( q );

    ClientConnection::client_conn_ptr client_ptr =
            ClientConnection::CreateConnection( io, 0 );

    boost::asio::connect( client_ptr->getSocket(), end_point );
#ifdef DEBUG
    std::cout << "Done with init stuff, handshake time," << std::endl;
#endif
    // Run through handshake with server.
    client_ptr->begin();
}

// Allegro loop goes in this function.
static void *AllegroThread( ALLEGRO_THREAD *thr, void *arg )
{
    ALLEGRO_DISPLAY     *display     = nullptr;
    ALLEGRO_EVENT_QUEUE *event_queue = nullptr;
    ALLEGRO_TIMER       *timer       = nullptr;
    bool                 render      = true;

    if( !al_install_mouse() )
    {
        std::cerr << "Error initializing Allegro mouse driver." << std::endl;
        return nullptr;
    }

    if( !( timer = al_create_timer( 1.0 / 30 ) ) )
    {
        std::cerr << "Error initializing Allegro timer." << std::endl;
        return nullptr;
    }

    if( !( display = al_create_display( 800, 600 ) ) )
    {
        std::cerr << "Error initializing Allegro dispay driver." << std::endl;
        al_destroy_timer( timer );
        return nullptr;
    }

    if( !( event_queue = al_create_event_queue() ) )
    {
        std::cerr << "Error initializing Allegro event queue." << std::endl;
        al_destroy_display( display );
        al_destroy_timer( timer );
        return nullptr;
    }

    al_register_event_source( event_queue, al_get_display_event_source( display ) );
    al_register_event_source( event_queue, al_get_timer_event_source( timer ) );
    al_register_event_source( event_queue, al_get_mouse_event_source() );

    al_clear_to_color( al_map_rgb( 255, 255, 255 ) );
    al_start_timer( timer );

    while( 1 )
    {
        ALLEGRO_EVENT event;
        al_wait_for_event( event_queue, &event );

        if( event.type == ALLEGRO_EVENT_TIMER )
        {
            render = true;
        }
        else if( event.type == ALLEGRO_EVENT_DISPLAY_CLOSE ||
                 event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN )
        {
            break;
        }

        if( render && al_is_event_queue_empty( event_queue ) )
        {
            render = false;

            // Rendering code goes here.
            // Threading this off too would be cool as shit.
            // Maybe I just have a thread fetish.

            al_flip_display();
        }
    }

    al_destroy_timer( timer );
    al_destroy_display( display );
    al_destroy_event_queue( event_queue );

    return nullptr;
}

int c_main( int argc, char **argv )
{
    ALLEGRO_THREAD *allegro_thread = nullptr;

    try
    {
        boost::asio::io_service io;
        PetEscapeClient pec( io, argv[2]);

        // At this point, we need to kick into a thread
        // to do Allegro stuff.

        if( !al_init() )
        {
            std::cerr << "Failed to initialize Allegro.\n";
            return -1;
        }

        allegro_thread = al_create_thread( AllegroThread, nullptr );
        al_start_thread( allegro_thread );

        io.run();
    }
    catch( std::exception &e )
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    if( allegro_thread ) al_destroy_thread( allegro_thread );

    return 0;
}

}}}
