
#include "petescape/core/client/client.h"
#include "petescape/core/client/client_resources.h"
#include "petescape/core/ObjectRenderer.h"
#include "petescape/core/core_defs.h"
#include "petescape/core/GameMap.h"
#include "petescape/core/BlockMap.h"

#include "petescape/networking/common/net_struct.h"

#include "launcher.h"

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <boost/foreach.hpp>

#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>

#include <map>
#include <cstdlib>

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
ALLEGRO_TIMER                *timer;
boost::asio::io_service       client_io_service;
boost::asio::ip::tcp::socket *socket;

network_packet                      input;

uint8_t                       client_id;

uint8_t                       map_length;
uint8_t                       map_height;
GameMap                      *map;
BlockMap                     *block_map;

NewGameState                  game_state;

int                           num_map_packets_recieved;
char server_ip_address[ 20 ];
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
        packet.head.sender_id = client_id;

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

    void handlePacket( const network_packet *packet )
    {
        packet_list new_packet;

        switch( packet->head.opcode )
        {
        case S_INFO:
        {
            MESSAGE( "CLIENT: Recieved S_INFO" );
            client_id = packet->data.s_info.client_id;

            game_state = State_LoadMap;

            MESSAGE( "CLIENT: Sending C_REQUEST_MAP" );
            NetOps.async_write( new_packet, C_REQUEST_MAP );
        } break;
        case S_MAP_HEADER:
        {
            MESSAGE( "CLIENT: Recieved S_MAP_HEADER" );
            map_length = packet->data.s_map_header.stage_length;
            map_height = packet->data.s_map_header.stage_height;

            MESSAGE( "CLIENT: MapSize = " << (int) map_length << " x " << (int) map_height );

            //a little rough
            map = new GameMap( map_height, map_length );
        } break;

        case S_MAP_DATA:
        {
            MESSAGE( "CLIENT: Recieved S_MAP_DATA " << ((int)packet->data.s_map_data.packet_number));
            map->addChunk( packet->data.s_map_data );
        } break;

        case S_SENT_MAP:
        {
            MESSAGE( "CLIENT: Recieved S_SENT_MAP" );
            // all packets received
            block_map = new BlockMap( *map );

            game_state = State_LoadObjects;

            MESSAGE( "CLIENT: Sending C_REQUEST_OBJS" );
            NetOps.async_write( new_packet, C_REQUEST_OBJS );
        }

        case S_SENT_OBJS:
        {
            MESSAGE( "CLIENT: Recieved S_SENT_OBJS" );

            game_state = State_Playing;
        } break;

        case O_UPDATE:
        {
//            MESSAGE( "CLIENT: Recieved O_UPDATE" );
            block_map->updateObject( &packet->data.o_update );
        } break;

        case O_DESTORY:
        {
            MESSAGE( "CLIENT: Recieved O_DESTROY" );
            block_map->destroyObject( &packet->data.o_destroy );
        } break;

        default:
        {
            // Do nothing.
            MESSAGE( "Do nothing." );
        } break;
        }
    }

    // cut load sprite map

};

namespace {
GameOps_   GameOps;

void load_images()
{
    static bool loaded = false;

    if( loaded )
        return;

    // Load bitmaps that we need.
    play_solo_bitmap = al_load_bitmap( "assets/welcome_screen/play_solo.bmp" );
    host_game_bitmap = al_load_bitmap( "assets/welcome_screen/host_game.bmp" );
    join_game_bitmap = al_load_bitmap( "assets/welcome_screen/join_game.bmp" );
    quit_game_bitmap = al_load_bitmap( "assets/welcome_screen/quit_game.bmp" );

    if( !play_solo_bitmap ||
        !host_game_bitmap ||
        !join_game_bitmap ||
        !quit_game_bitmap)
    {
        MESSAGE( "Unable to load welcome screen bitmaps. Exiting." );
        exit( 2 );
    }

    play_solo_bounds.x = al_get_display_width( display ) / 2 - al_get_bitmap_width( play_solo_bitmap ) / 2;
    play_solo_bounds.y = al_get_display_height( display ) / 5 * 1 - al_get_bitmap_height( play_solo_bitmap ) / 2;
    play_solo_bounds.width = al_get_bitmap_width( play_solo_bitmap );
    play_solo_bounds.height = al_get_bitmap_height( play_solo_bitmap );

    host_game_bounds.x = al_get_display_width( display ) / 2 - al_get_bitmap_width( host_game_bitmap ) / 2;
    host_game_bounds.y = al_get_display_height( display ) / 5 * 2 - al_get_bitmap_height( host_game_bitmap ) / 2;
    host_game_bounds.width = al_get_bitmap_width( host_game_bitmap );
    host_game_bounds.height = al_get_bitmap_height( host_game_bitmap );

    join_game_bounds.x = al_get_display_width( display ) / 2 - al_get_bitmap_width( join_game_bitmap ) / 2;
    join_game_bounds.y = al_get_display_height( display ) / 5 * 3 - al_get_bitmap_height( join_game_bitmap ) / 2;
    join_game_bounds.width = al_get_bitmap_width( join_game_bitmap );
    join_game_bounds.height = al_get_bitmap_height( join_game_bitmap );

    quit_game_bounds.x = al_get_display_width( display ) / 2 - al_get_bitmap_width( quit_game_bitmap ) / 2;
    quit_game_bounds.y = al_get_display_height( display ) / 5 * 4 - al_get_bitmap_height( quit_game_bitmap ) / 2;
    quit_game_bounds.width = al_get_bitmap_width( quit_game_bitmap );
    quit_game_bounds.height = al_get_bitmap_height( quit_game_bitmap );

    loaded = true;
}

}

void unload_images()
{
    al_destroy_bitmap( play_solo_bitmap );
    al_destroy_bitmap( host_game_bitmap );
    al_destroy_bitmap( join_game_bitmap );
    al_destroy_bitmap( quit_game_bitmap );
}

void render_welcome_state()
{
    al_draw_bitmap( play_solo_bitmap,
                    play_solo_bounds.x,
                    play_solo_bounds.y,
                    0 );

    al_draw_bitmap( host_game_bitmap,
                    host_game_bounds.x,
                    host_game_bounds.y,
                    0 );

    al_draw_bitmap( join_game_bitmap,
                    join_game_bounds.x,
                    join_game_bounds.y,
                    0 );

    al_draw_bitmap( quit_game_bitmap,
                    quit_game_bounds.x,
                    quit_game_bounds.y,
                    0 );

}

int c_main( int /*argc*/, char **argv )
{
    Launcher *launcher = new Launcher();

    boost::asio::io_service::work work( client_io_service );
    boost::thread io_thread( boost::bind( &boost::asio::io_service::run, &client_io_service ) );

    client_queue = nullptr;
    display = nullptr;
    timer = nullptr;
    block_map = nullptr;
    bool key[5] = { false, false, false, false, false}; //up, left, right down, a

    game_state = State_Welcome;
    memset( server_ip_address, '\0', sizeof( server_ip_address ) );

    try
    {
        bool should_exit = false;
        bool redraw      = false;

        if( !al_init() )
        {
            MESSAGE( "CLIENT: Error initializing Allegro." );
            return -1;
        }

        ALLEGRO_PATH *path = al_get_standard_path( ALLEGRO_RESOURCES_PATH );
        al_change_directory( al_path_cstr( path, '/' ) );

        if(!al_install_keyboard()) {
            MESSAGE( "CLIENT: Error initializing Allegro keyboard driver." );
            return -1;
        }

        if( !al_install_mouse() )
        {
            MESSAGE( "CLIENT: Error initializing Allegro mouse driver." );
            return -2;
        }

        if( !( client_queue = al_create_event_queue() ) )
        {
            MESSAGE( "CLIENT: Error initializing Allegro Event Queue." );
            return -3;
        }

        if( !( timer = al_create_timer( 1.0 / FPS ) ) )
        {
            MESSAGE( "CLIENT: Error initializing Allegro timer." );
            return -4;
        }

        if( !( display = al_create_display( 800, 608 ) ) )
        {
            MESSAGE( "CLIENT: Error initializing Allegro dispay driver." );
            al_destroy_timer( timer );
            return -5;
        }

        al_init_font_addon();
        al_init_ttf_addon();

        if( !( default_font = al_load_ttf_font( "assets/fonts/FRABK.TTF", 35, 0 ) ) )
        {
            MESSAGE( "CLIENT: Error loading font." );
            al_destroy_timer( timer );
            al_destroy_display( display );
            return -6;
        }

        al_init_image_addon();
        al_init_primitives_addon();

        // Load the images.
        MESSAGE( "CLIENT: Loading images..." );
        block_map->load_images();
        MESSAGE( "CLIENT: Loaded imgaes." );

        al_register_event_source( client_queue, al_get_display_event_source( display ) );
        al_register_event_source( client_queue, al_get_timer_event_source( timer ) );
        al_register_event_source( client_queue, al_get_mouse_event_source() );
        al_register_event_source(client_queue, al_get_keyboard_event_source());

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
            case ALLEGRO_EVENT_TIMER: {

                static uint64_t counter = 0;

                if( game_state == State_Playing )
                {
                    if( block_map->getPlayers()[ client_id ] == nullptr )
                        continue;

                    if( key[ KEY_UP ] )
                    {
                        block_map->getPlayers()[ client_id ]->start_jump();
                    }

                    if( key[ KEY_LEFT ] )
                    {
                        block_map->getPlayers()[ client_id ]->start_move_left();
                    }

                    if( key[ KEY_RIGHT ] )
                    {
                        block_map->getPlayers()[ client_id ]->start_move_right();
                    }

                    if( !key[ KEY_LEFT ] && !key[ KEY_RIGHT ] )
                    {
                        block_map->getPlayers()[ client_id ]->stop_moving();
                    }

                    if( key[ KEY_A ] )
                    {
                        printf("A pressed");
                    }

                    block_map->getPlayers()[ client_id ]->update();
                    ++counter;

                    // For now throw a packet out every frame. Not laggy at all.
                    packet_list packet;
                    packet.o_update.id = client_id;
                    packet.o_update.type = PlayerType;
                    packet.o_update.facing = block_map->getPlayers()[ client_id ]->get_facing();
                    packet.o_update.walk_phase = block_map->getPlayers()[ client_id ]->get_walk_phase();
                    packet.o_update.x = (uint32_t)( block_map->getPlayers()[ client_id ]->getX() );
                    packet.o_update.y = (uint32_t)( block_map->getPlayers()[ client_id ]->getY() );

                    NetOps.async_write( packet, O_UPDATE );
                }

                redraw = true;

            } break;

            case ALLEGRO_EVENT_KEY_DOWN: {

                if( game_state == State_Playing )
                {
                    switch(event.keyboard.keycode)
                    {
                    case ALLEGRO_KEY_UP:
                        key[KEY_UP] = true;
                        break;

                    case ALLEGRO_KEY_DOWN:
                        key[KEY_DOWN] = true;
                        break;

                    case ALLEGRO_KEY_LEFT:
                        key[KEY_LEFT] = true;
                        break;

                    case ALLEGRO_KEY_RIGHT:
                        key[KEY_RIGHT] = true;
                        break;

                    case ALLEGRO_KEY_A:
                        key[KEY_A] = true;
                        break;
                    }
                }

            } break;

            case ALLEGRO_EVENT_KEY_UP: {
               if( game_state == State_Playing )
               {
                   switch(event.keyboard.keycode)
                   {
                   case ALLEGRO_KEY_UP:
                       key[KEY_UP] = false;
                       break;

                   case ALLEGRO_KEY_DOWN:
                       key[KEY_DOWN] = false;
                       break;

                   case ALLEGRO_KEY_LEFT:
                       key[KEY_LEFT] = false;
                       break;

                   case ALLEGRO_KEY_RIGHT:
                       key[KEY_RIGHT] = false;
                       break;

                   }
               }

           } break;

            case ALLEGRO_EVENT_MOUSE_BUTTON_DOWN: {
                int x, y;
                packet_list padding;

                x = event.mouse.x;
                y = event.mouse.y;

                switch( game_state )
                {
                case State_Welcome: {
                    // Did they click on the first box?
                    if( IS_WITHIN( play_solo_bounds, x, y ) )
                    {
                        launcher->start( "PetEscape --server" );

                        if( socket == nullptr )
                        {
                            game_state = State_Init;

                            boost::asio::ip::tcp::resolver tcp_resolver( client_io_service );
                            boost::asio::ip::tcp::resolver::query tcp_query( "127.0.0.1", "2001" );
                            boost::asio::ip::tcp::resolver::iterator tcp_endpoint;

                            tcp_endpoint = tcp_resolver.resolve( tcp_query );

                            socket = new boost::asio::ip::tcp::socket( client_io_service );
                            boost::asio::connect( *socket, tcp_endpoint );

                            // padding.c_hello.solo = 1;
                            MESSAGE( "CLIENT: Sending C_HELLO" );
                            NetOps.async_write( padding, C_HELLO );

                            NetOps.async_read();
                        }
                    }

                    // Did they click on the second box?
                    // TODO: Eventually, this will differ from PlaySolo,
                    // because play solo won't allow more than one connection.
                    if( IS_WITHIN( host_game_bounds, x, y ) )
                    {
                        launcher->start( "PetEscape --server" );

                        if( socket == nullptr )
                        {
                            game_state = State_Init;

                            boost::asio::ip::tcp::resolver tcp_resolver( client_io_service );
                            boost::asio::ip::tcp::resolver::query tcp_query( "127.0.0.1", "2001" );
                            boost::asio::ip::tcp::resolver::iterator tcp_endpoint;

                            tcp_endpoint = tcp_resolver.resolve( tcp_query );

                            socket = new boost::asio::ip::tcp::socket( client_io_service );
                            boost::asio::connect( *socket, tcp_endpoint );

                            // padding.c_hello.solo = 0;
                            MESSAGE( "CLIENT: Sending C_HELLO" );
                            NetOps.async_write( padding, C_HELLO );

                            NetOps.async_read();
                        }
                    }

                    // Did they click on the third box?
                    if( IS_WITHIN( join_game_bounds, x, y ) )
                    {
                        char *ip = launcher->getIP();

                        if( ip != nullptr )
                        {
                            strcpy( server_ip_address, ip );
                            free( ip );
                        }

                        if( ( ip != nullptr ) && ( socket == nullptr ) )
                        {
                            game_state = State_Init;

                            boost::asio::ip::tcp::resolver tcp_resolver( client_io_service );
                            boost::asio::ip::tcp::resolver::query tcp_query( server_ip_address, "2001" );
                            boost::asio::ip::tcp::resolver::iterator tcp_endpoint;

                            tcp_endpoint = tcp_resolver.resolve( tcp_query );

                            socket = new boost::asio::ip::tcp::socket( client_io_service );
                            boost::asio::connect( *socket, tcp_endpoint );

                            // padding.c_hello.solo = 0;
                            MESSAGE( "CLIENT: Sending C_HELLO" );
                            NetOps.async_write( padding, C_HELLO );

                            NetOps.async_read();
                        }
                    }

                    // Did they click on the fourth box?
                    if( IS_WITHIN( quit_game_bounds, x, y ) )
                    {
                        should_exit = true;
                    }

                } break;
                }
            } break;

            case NETWORK_RECV: {
                // Pass the packet off to the packet handler.
                GameOps.handlePacket( (network_packet *)event.user.data1 );

                // Clean up memory usage.
                delete (network_packet *)event.user.data1;
            } break;

            case NETWORK_CLOSE: {
                MESSAGE( "CLIENT: Recieving event NETWORK_CLOSE" );
            } break;

            case ALLEGRO_EVENT_DISPLAY_CLOSE: {
                packet_list c_close;

                MESSAGE( "CLIENT: Recieving event ALLEGRO_EVENT_DISPLAY_CLOSE" );

                if( socket != nullptr )
                {
                    c_close.c_close.client_id = client_id;
                    NetOps.async_write( c_close, C_CLOSE );
                }

                should_exit = true;
            } break;
            }

            if( redraw && al_is_event_queue_empty( client_queue ) )
            {
                // Set screen to white.
                al_clear_to_color( al_map_rgb( 255, 255, 255 ) );

                switch( game_state )
                {
                case State_Welcome:
                    render_welcome_state();
                    break;

                case State_Playing:
                    block_map->render_playing_state();
                    break;
                }

                al_flip_display();
                redraw = false;
            }
        }

        client_io_service.stop();
        io_thread.join();

        launcher->kill();

        al_destroy_path(path);

        block_map->unload_images();

        if( default_font ) al_destroy_font( default_font );
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
