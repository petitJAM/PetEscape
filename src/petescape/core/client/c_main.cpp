
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
ALLEGRO_DISPLAY              *display;
ALLEGRO_TIMER                *timer;
boost::asio::io_service       client_io_service;
boost::asio::ip::tcp::socket *socket;

network_packet                      input;
std::map<uint32_t, GameObject *>    objs;
std::map<uint32_t, PlayerObject *>  players;

uint8_t                       client_id;

uint8_t                       map_length;
uint8_t                       map_height;
GameMap                      *map;
BlockMap                     *block_map;

GameState                     game_state;

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

    void updateObject( const update_obj *data )
    {
        switch( data->type )
        {
        case PlayerType:
            players[ data->id ]->setX( data->x );
            players[ data->id ]->setY( data->y );
            players[ data->id ]->set_facing( data->facing );
        break;
        case OtherType:
            objs[ data->id ]->setX( data->x );
            objs[ data->id ]->setY( data->y );
        break;
        default:
            MESSAGE( "Unsupported Type. Defaulting to basic type." );
        break;
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

            MESSAGE( "Creating object." );

            switch( data->type )
            {
            case PlayerType:
                MESSAGE( "Got new player" );
                obj = PlayerObject::CreatePlayer( data->id );
                players[ data->id ] = static_cast<PlayerObject*>(obj);

                obj->setRenderer( new petescape::core::PlayerRenderer( character_bitmaps[ data->id ] ) );
                MESSAGE( "Done with new player" );
            break;
            case OtherType:
                obj = GameObject::CreateGameObject( data->id );
                objs[ data->id ] = obj;
                obj->setRenderer( new petescape::core::PoorRenderer );
            break;
            default:
                MESSAGE( "Unsupported Type. Defaulting to basic type." );
                obj = GameObject::CreateGameObject( data->id );
                objs[ data->id ] = obj;
            break;
            }

            obj->setX( data->x );
            obj->setY( data->y );

            obj->put_in_map( block_map );
        }
    }

    void destroyObject( const destroy_obj *data )
    {
        objs.erase( data->id );
    }

    void handlePacket( const network_packet *packet )
    {
        packet_list new_packet;

        MESSAGE( "Packet?" );
        MESSAGE( "Opcode? " << packet->head.opcode );

        switch( packet->head.opcode )
        {
        case S_INFO:
            client_id = packet->data.s_info.client_id;
            NetOps.async_write( new_packet, C_READY );
            MESSAGE( "recieved S_INFO, sending C_READY" );
        break;
        case S_MAP_HEADER:
            map_length = packet->data.s_map_header.stage_length;
            map_height = packet->data.s_map_header.stage_height;
            MESSAGE( "recieved S_MAP_HEADER, sending C_REQUEST_MAP" );
            MESSAGE("expecting map of size " << (int) map_length << " x " << (int) map_height);

            //a little rough
            num_map_packets_recieved = 0;
            map = new GameMap(map_height, map_length);

            NetOps.async_write(new_packet, C_REQUEST_MAP);
        break;

        case S_READY:
        {
            MESSAGE( "READY!" );
            game_state = PlayingState;
        }break;

        case S_MAP_DATA:
        {
            map->addChunk(packet->data.s_map_data);

            MESSAGE("recieved S_MAP_DATA " << ((int)packet->data.s_map_data.packet_number));
            num_map_packets_recieved++;

            // all packets received
            if(num_map_packets_recieved >= ((map_length * map_height) / MAP_PACKET_SIZE)){
                //map->display();
                block_map = new BlockMap(*map);
                //block_map->display();

                NetOps.async_write(new_packet, C_BUILD_OBJECTS);
                MESSAGE( "sending C_BUILD_OBJECTS");
            }
        }
        break;
        case O_INTRODUCE:
            MESSAGE( "recieved O_INTRODUCE" );
            genObject( &packet->data.o_introduce );
        break;
        case O_UPDATE:
            MESSAGE( "recieved O_UPDATE" );
            updateObject( &packet->data.o_update );
        break;
        case O_DESTORY:
            MESSAGE( "recieved O_DESTROY" );
            destroyObject( &packet->data.o_destroy );
        break;

        default:
            // Do nothing.
            MESSAGE( "Do nothing." );
        break;
        }
    }

    ALLEGRO_BITMAP** load_sprite_map( ALLEGRO_BITMAP *master,
                                      int32_t t_w,
                                      int32_t t_h,
                                      int32_t &tile_count )
    {
        int img_w = al_get_bitmap_width( master );
        int img_h = al_get_bitmap_height( master );
        int h_count = img_w / t_w;
        int v_count = img_h / t_h;
        int index = 0;

        ALLEGRO_BITMAP *original_target = al_get_target_bitmap();

        ALLEGRO_BITMAP **tiles =
                ( ALLEGRO_BITMAP** )malloc( sizeof( ALLEGRO_BITMAP* ) * h_count * v_count );

        if( tiles == nullptr )
            goto err;

        for( int i = 0; i < v_count; ++i )
        {
            for( int j = 0; j < h_count; ++j )
            {
                tiles[ index ] = al_create_bitmap( t_w, t_h );

                if( tiles[ index ] != nullptr )
                {
                    al_set_target_bitmap( tiles[ index ] );
                    al_draw_bitmap( master, -( j * t_w ), -( i * t_h ), 0 );
                }
                else
                {
                    index = 0;
                    free( tiles );
                    goto err;
                }

                ++index;
            }
        }

        al_set_target_bitmap( original_target );
err:
        tile_count = index;

        return tiles;
    }

};

namespace {
GameOps_   GameOps;
}

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

    // Load character that we need.
    // wtf why did you change the name to cha.bmp... that doesn't even make sense.
    ALLEGRO_BITMAP *char_map = al_load_bitmap( "assets/character/cha.bmp" );
    int tile_count = 0;
    ALLEGRO_BITMAP **characters = GameOps.load_sprite_map( char_map, 32, 64, tile_count );

    if( tile_count != ( 8 * 4 ) )
    {
        MESSAGE( "Didn't load correct image count... " << tile_count );
        exit( 1 );
    }

    if( characters != nullptr )
    {
        for( int i = 0; i < 4; ++i )
        {
            for( int j = 0; j < 8; ++j )
            {
                character_bitmaps[ i ][ j ] = characters[ i * 8 + j ];
            }

            current_char_bitmap[ i ] = characters[ i * 8 ];
            current_char_bounds[ i ].x = 0;
            current_char_bounds[ i ].y = 0;
            current_char_bounds[ i ].width = 32;
            current_char_bounds[ i ].height = 64;
        }
    }
    else
    {
        MESSAGE( "Could not load character images..." );
        exit( 1 );
    }

    al_destroy_bitmap( char_map );

    loaded = true;
}

void unload_images()
{
    if( character_bitmaps != nullptr )
    {
        for( int i = 0; i < 4; ++i )
        {
            for( int j = 0; j < 8; ++j )
            {
                al_destroy_bitmap( character_bitmaps[ i ][ j ] );
            }
        }
    }

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

void render_playing_state()
{
    // Render the background.

    for( int i = 0; i < block_map->getLength(); ++i )
    {
        for( int j = 0; j < block_map->getHeight(); ++j )
        {
            if( block_map->getBlock( i, j ).getBlockType() )
                al_draw_filled_rectangle( i * 32, j * 32, (i+1) * 32, (j+1) * 32, al_map_rgb( 127,127,127 ) );
            else
                al_draw_filled_rectangle( i * 32, j * 32, (i+1) * 32, (j+1) * 32, al_map_rgb( 255,255,255 ) );
        }
    }

    // Rendering code goes here
    BOOST_FOREACH( m_element tmp, objs )
    {
        ((GameObject*)(tmp.second))->render();
    }

    BOOST_FOREACH( m_element tmp, players )
    {
        ((GameObject*)(tmp.second))->render();
    }
}

void render_pause_state()
{

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
    bool key[4] = { false, false, false, false};

    game_state = WelcomeState;
    memset( server_ip_address, '\0', sizeof( server_ip_address ) );

    try
    {
        bool should_exit = false;
        bool redraw      = false;

        if( !al_init() )
        {
            MESSAGE( "Failed to initialize Allegro." );
            return -1;
        }

        ALLEGRO_PATH *path = al_get_standard_path( ALLEGRO_RESOURCES_PATH );
        al_change_directory( al_path_cstr( path, '/' ) );

        if(!al_install_keyboard()) {
            MESSAGE( "failed to initialize the keyboard!" );
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

        if( !( timer = al_create_timer( 1.0 / FPS ) ) )
        {
            MESSAGE( "Error initializing Allegro timer." );
            return -4;
        }

        if( !( display = al_create_display( 800, 608 ) ) )
        {
            MESSAGE( "Error initializing Allegro dispay driver." );
            al_destroy_timer( timer );
            return -5;
        }

        al_init_font_addon();
        al_init_ttf_addon();

        if( !( default_font = al_load_ttf_font( "assets/fonts/FRABK.TTF", 35, 0 ) ) )
        {
            MESSAGE( "Error loading font." );
            al_destroy_timer( timer );
            al_destroy_display( display );
            return -6;
        }

        al_init_image_addon();
        al_init_primitives_addon();

        // Load the images.
        MESSAGE( "Loading images..." );
        load_images();
        MESSAGE( "Loaded imgaes." );

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

                if( game_state == PlayingState )
                {
                    if( players[ client_id ] == nullptr )
                        continue;

                    if( key[ KEY_UP ] )
                    {
                        players[ client_id ]->start_jump();
                    }

                    if( key[ KEY_LEFT ] )
                    {
                        players[ client_id ]->start_move_left();
                    }

                    if( key[ KEY_RIGHT ] )
                    {
                        players[ client_id ]->start_move_right();
                    }

                    if( !key[ KEY_LEFT ] && !key[ KEY_RIGHT ] )
                    {
                        players[ client_id ]->stop_moving();
                    }

                    players[ client_id ]->update();
                    ++counter;

//                    if( counter % 4 == 0 )
//                    {
                    // For now throw a packet out every frame. Not laggy at all.
                    packet_list packet;
                    packet.o_update.id = client_id;
                    packet.o_update.type = PlayerType;
                    packet.o_update.facing = players[ client_id ]->get_facing();
                    packet.o_update.x = (uint32_t)( players[ client_id ]->getX() );
                    packet.o_update.y = (uint32_t)( players[ client_id ]->getY() );

                    NetOps.async_write( packet, O_UPDATE );
//                    }
                }

                redraw = true;

            } break;

            case ALLEGRO_EVENT_KEY_DOWN: {

                if( game_state == PlayingState )
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

                    }
                }

            } break;

            case ALLEGRO_EVENT_KEY_UP: {
               if( game_state == PlayingState )
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
                case WelcomeState: {
                    // Did they click on the first box?
                    if( IS_WITHIN( play_solo_bounds, x, y ) )
                    {
                        MESSAGE( "CLICKED PLAY SOLO" );

                        launcher->start( "PetEscape --server" );

                        if( socket == nullptr )
                        {
                            boost::asio::ip::tcp::resolver tcp_resolver( client_io_service );
                            boost::asio::ip::tcp::resolver::query tcp_query( "127.0.0.1", "2001" );
                            boost::asio::ip::tcp::resolver::iterator tcp_endpoint;

                            tcp_endpoint = tcp_resolver.resolve( tcp_query );

                            socket = new boost::asio::ip::tcp::socket( client_io_service );
                            boost::asio::connect( *socket, tcp_endpoint );
                            MESSAGE( "Connected." );

                            // padding.c_hello.solo = 1;
                            NetOps.async_write( padding, C_HELLO );
                            MESSAGE( "sending C_HELLO" );

                            NetOps.async_read();
                        }

                        game_state = SetupState;
                    }

                    // Did they click on the second box?
                    // TODO: Eventually, this will differ from PlaySolo,
                    // because play solo won't allow more than one connection.
                    if( IS_WITHIN( host_game_bounds, x, y ) )
                    {
                        MESSAGE( "CLICKED HOST GAME" );

                        launcher->start( "PetEscape --server" );

                        if( socket == nullptr )
                        {
                            boost::asio::ip::tcp::resolver tcp_resolver( client_io_service );
                            boost::asio::ip::tcp::resolver::query tcp_query( "127.0.0.1", "2001" );
                            boost::asio::ip::tcp::resolver::iterator tcp_endpoint;

                            tcp_endpoint = tcp_resolver.resolve( tcp_query );

                            socket = new boost::asio::ip::tcp::socket( client_io_service );
                            boost::asio::connect( *socket, tcp_endpoint );
                            MESSAGE( "Connected." );

                            // padding.c_hello.solo = 0;
                            NetOps.async_write( padding, C_HELLO );
                            MESSAGE( "sending C_HELLO" );

                            NetOps.async_read();
                        }

                        game_state = SetupState;
                    }

                    // Did they click on the third box?
                    if( IS_WITHIN( join_game_bounds, x, y ) )
                    {
                        MESSAGE( "CLICKED JOIN GAME" );

                        char *ip = launcher->getIP();

                        if( ip != nullptr )
                        {
                            strcpy( server_ip_address, ip );
                            free( ip );

                            MESSAGE( server_ip_address );
                        }
                        else
                        {
                            MESSAGE( "Nevermind." );
                        }

                        if( ( ip != nullptr ) && ( socket == nullptr ) )
                        {
                            boost::asio::ip::tcp::resolver tcp_resolver( client_io_service );
                            boost::asio::ip::tcp::resolver::query tcp_query( server_ip_address, "2001" );
                            boost::asio::ip::tcp::resolver::iterator tcp_endpoint;

                            tcp_endpoint = tcp_resolver.resolve( tcp_query );

                            socket = new boost::asio::ip::tcp::socket( client_io_service );
                            boost::asio::connect( *socket, tcp_endpoint );
                            MESSAGE( "Connected." );

                            // padding.c_hello.solo = 0;
                            NetOps.async_write( padding, C_HELLO );
                            MESSAGE( "sending C_HELLO" );

                            NetOps.async_read();

                            game_state = SetupState;
                        }
                    }

                    // Did they click on the fourth box?
                    if( IS_WITHIN( quit_game_bounds, x, y ) )
                    {
                        MESSAGE( "CLICKED QUIT GAME" );
                        should_exit = true;
                    }

                } break;
                }
            } break;

            case NETWORK_RECV: {
                MESSAGE( "recieving event NETWORK_RECV" );

                // Pass the packet off to the packet handler.
                GameOps.handlePacket( (network_packet *)event.user.data1 );

                // Clean up memory usage.
                delete (network_packet *)event.user.data1;
            } break;

            case NETWORK_CLOSE: {
                MESSAGE( "recieving event NETWORK_CLOSE" );
            } break;

            case ALLEGRO_EVENT_DISPLAY_CLOSE: {
                packet_list padding;

                MESSAGE( "recieving event ALLEGRO_EVENT_DISPLAY_CLOSE" );

                if( socket != nullptr )
                    NetOps.async_write( padding, C_CLOSE );

                should_exit = true;
            } break;
            }

            if( redraw && al_is_event_queue_empty( client_queue ) )
            {
                // Set screen to white.
                al_clear_to_color( al_map_rgb( 255, 255, 255 ) );

                switch( game_state )
                {
                case WelcomeState:
                    render_welcome_state();
                    break;

                case PlayingState:
                    if( block_map == nullptr )
                        continue;

                    render_playing_state();
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

        unload_images();

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
