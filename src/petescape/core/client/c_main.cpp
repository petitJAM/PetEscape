
#include "petescape/core/client/client.h"
#include "petescape/networking/client/ClientConnection.h"
#include "petescape/networking/common/net_struct.h"

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <allegro5/allegro.h>


namespace petescape {
namespace core {
namespace client {

struct ALLEGRO_THREAD_ARGS
{
    PetEscapeClient *client;
};

struct NETWORK_THREAD_ARGS
{
    ALLEGRO_EVENT_SOURCE *event_src;
    ALLEGRO_EVENT_QUEUE  *net_queue;
    PetEscapeClient      *client;
};

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

    m_client_ptr = ClientConnection::CreateConnection( io, 0 );

    boost::asio::connect( m_client_ptr->getSocket(), end_point );
#ifdef DEBUG
    std::cout << "Done with init stuff, handshake time," << std::endl;
#endif
    // Run through handshake with server.
    m_client_ptr->begin();
}

void PetEscapeClient::introduceObject( const introduce_obj *data )
{
    if( this->m_objects.count( data->id ) )
    {
        updateObject( data );
    }
    else
    {
        GameObject *obj = GameObject::CreateGameObject( data->id );
        obj->setX( data->x );
        obj->setY( data->y );

        this->m_objects[ data->id ] = obj;
    }
}

void PetEscapeClient::updateObject( const update_obj *data )
{
    if( this->m_objects.count( data->id ) == 1 )
    {
        this->m_objects[ data->id ]->setX( data->x );
        this->m_objects[ data->id ]->setY( data->y );
    }
}

void PetEscapeClient::destoryObject( const destroy_obj *data )
{
    this->m_objects.erase( data->id );
}

// Allegro loop goes in this function.
static void *AllegroThread( ALLEGRO_THREAD *thr, void *arg )
{
    struct NETWORK_THREAD_ARGS *t_args      = nullptr;
    ALLEGRO_DISPLAY            *display     = nullptr;
    ALLEGRO_EVENT_QUEUE        *event_queue = nullptr;
    ALLEGRO_TIMER              *timer       = nullptr;
    bool                        render      = true;

    t_args = (struct NETWORK_THREAD_ARGS *) arg;

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

// This thread handles all the communication between the
// boost::asio code and Allegro. It parses the data from
// the server that has been thrown into an event queue
// in its raw form. It dispatches the server instructions,
// and sends off a reply if required.
static void* NetHandleThread( ALLEGRO_THREAD *thr, void *arg )
{
    struct NETWORK_THREAD_ARGS *t_args = (struct NETWORK_THREAD_ARGS *) arg;

    while( 1 )
    {
        ALLEGRO_EVENT event;
        al_wait_for_event( t_args->net_queue, &event );

        if( event.type == 512 )
        {
            // Got message from server. do stuff.
        }
    }

    return nullptr;
}

int c_main( int argc, char **argv )
{
    ALLEGRO_THREAD       *allegro_thread = nullptr;
    ALLEGRO_THREAD       *network_thread = nullptr;
    ALLEGRO_EVENT_QUEUE  *net_queue      = nullptr;
    PetEscapeClient      *client         = nullptr;

    ALLEGRO_EVENT_SOURCE    server_evt_src;
    boost::asio::io_service io_service;
    struct NETWORK_THREAD_ARGS n_args;
    struct ALLEGRO_THREAD_ARGS a_args;

    try
    {
        // Start Allegro (need for threads)
        if( !al_init() )
        {
            std::cerr << "Failed to initialize Allegro.\n";
            return -1;
        }

        net_queue = al_create_event_queue();
        al_init_user_event_source( &server_evt_src );
        al_register_event_source( net_queue, &server_evt_src );

        client = new PetEscapeClient( io_service, argv[2] );
        client->setNetworkEventSource( &server_evt_src );

        n_args.client = client;
        n_args.event_src = &server_evt_src;
        n_args.net_queue = net_queue;

        a_args.client = client;

        allegro_thread = al_create_thread( AllegroThread, &a_args );
        network_thread = al_create_thread( NetHandleThread, &n_args );
        al_start_thread( allegro_thread );
        al_start_thread( network_thread );

        io_service.run();
    }
    catch( std::exception &e )
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    if( allegro_thread ) al_destroy_thread( allegro_thread );
    if( network_thread ) al_destroy_thread( network_thread );
    if( net_queue )      al_destroy_event_queue( net_queue );

    return 0;
}

}}}
