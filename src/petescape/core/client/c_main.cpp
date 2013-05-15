#include <iostream>
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

NewGameState                  game_state;

uint8_t                       enemy1state; // magic right now , change it later
uint8_t                       enemy1facing;

uint8_t                       enemy2state;
uint8_t                       enemy2facing;

uint8_t                       enemy3state;
uint8_t                       enemy3facing;

uint32_t                      bullet_id = 0;

uint8_t                       num_map_packets_recieved;
uint32_t                      currentHp;


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
            if( players.count( data->id ) == 0 )
            {
                genObject( data );
            }
            else
            {
                players[ data->id ]->setX( data->x );
                players[ data->id ]->setY( data->y );
                players[ data->id ]->set_facing( data->facing );
                players[ data->id ]->set_walk_phase( data->walk_phase );
            }
        break;
        case BulletType:
            if( objs.count( data->id ) == 0 )
            {
                genObject( data );
            }
            else
            {
                objs[ data->id ]->setX( data->x );
                objs[ data->id ]->setY( data->y );
            }
        break;
        case OtherType:
            if( objs.count( data->id ) == 0 )
            {
                genObject( data );
            }
            else
            {
                objs[ data->id ]->setX( data->x );
                objs[ data->id ]->setY( data->y );
            }
        break;
        default:
            MESSAGE( "Unsupported Type. Defaulting to basic type." );
        break;
        }
    }

    void genObject( const update_obj *data)
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
        case BulletType:
            MESSAGE( "Got BulletType" );
            obj = Bullet::CreateBullet( data->id, data->p_id, data->x, data->y, data->facing );
            objs[ data->id ] = obj;
            obj->setRenderer( new petescape::core::PoorRenderer );
            MESSAGE( "Completed BulletType");
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

    void destroyObject( const destroy_obj *data )
    {
        switch( data->type )
        {
        case PlayerType:
            players.erase( data->id );
            break;

        case BulletType:
            objs.erase( data->id );
            break;

        case OtherType:
            objs.erase( data->id );
            break;

        default:
            MESSAGE( "CLIENT: Invalid object type." );
        }

        objs.erase( data->id );
    }

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
            Sleep(2);
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
            Sleep(2);
            NetOps.async_write( new_packet, C_REQUEST_OBJS );
        }

        case S_SENT_OBJS:
        {
            MESSAGE( "CLIENT: Recieved S_SENT_OBJS" );
            Sleep(2);
            game_state = State_Playing;
        } break;

        case O_UPDATE:
        {
//            MESSAGE( "CLIENT: Recieved O_UPDATE" );
            updateObject( &packet->data.o_update );
        } break;

        case O_DESTORY:
        {
            MESSAGE( "CLIENT: Recieved O_DESTROY" );
            destroyObject( &packet->data.o_destroy );
        } break;

        default:
        {
            // Do nothing.
            MESSAGE( "Do nothing." );
        } break;
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
    // lol =w = i change it to character happy? :)
    ALLEGRO_BITMAP *char_map = al_load_bitmap( "assets/character/character.bmp" );
    al_convert_mask_to_alpha(char_map,al_map_rgb(221,6,178));

    if (char_map == nullptr){
        MESSAGE( "Could not load character images..." );
        exit( 1 );
    }

    int tile_count = 0;
    ALLEGRO_BITMAP **characters = GameOps.load_sprite_map( char_map, 32, 64, tile_count );

    if( tile_count != ( 31 * 4 ) )
    {
        MESSAGE( "Didn't load correct image count... " << tile_count );
        exit( 1 );
    }

    if( characters != nullptr )
    {
        for( int i = 0; i < 4; ++i )
        {
            for( int j = 0; j < 31; ++j )
            {
                character_bitmaps[ i ][ j ] = characters[ i * 30 + j ];
            }

            current_char_bitmap[ i ] = characters[ i * 30 ];
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

    // Load enemy
    ALLEGRO_BITMAP *enemy_map = al_load_bitmap( "assets/character/enemy_map.bmp" );
    al_convert_mask_to_alpha(enemy_map,al_map_rgb(221,6,178));

    if (enemy_map == nullptr){
        MESSAGE( "Could not load character images..." );
        exit( 1 );
    }

    tile_count = 0;
    ALLEGRO_BITMAP **enemies = GameOps.load_sprite_map( enemy_map, 43, 64, tile_count );

    if( tile_count != ( 8 * 3 ) )
    {
        MESSAGE( "Didn't load correct image count... " << tile_count );
        exit( 1 );
    }

    if( enemies != nullptr )
    {
        for( int i = 0; i < 3; ++i )
        {
            for( int j = 0; j < 8; ++j )
            {
                enemy_bitmaps[ i ][ j ] = enemies[ i * 8 + j ];
            }

            current_enemy_bitmap[ i ] = enemies[ i * 8 ];
            current_enemy_bounds[ i ].x = 800;
            current_enemy_bounds[ i ].y = 480;
            current_enemy_bounds[ i ].width = 43;
            current_enemy_bounds[ i ].height = 64;
        }
    }
    else
    {
        MESSAGE( "Could not load character images..." );
        exit( 1 );
    }
    // end enemy

    // Load the tileset.
    ALLEGRO_BITMAP *tile_map = al_load_bitmap( "assets/tiles.bmp" );
    tile_count = 0;

    ALLEGRO_BITMAP **temp_tiles = GameOps.load_sprite_map( tile_map, 32, 32, tile_count );

    for( int i = 0; i < 25; ++i )
        tiles[ i ] = temp_tiles[ i ];

    if( tiles == nullptr )
    {
        MESSAGE( "ERROR LOADING TILES" );
        exit( -2 );
    }

    loaded = true;
}

void unload_images()
{
    if( character_bitmaps != nullptr )
    {
        for( int i = 0; i < 4; ++i )
        {
            for( int j = 0; j < 30; ++j )
            {
                al_destroy_bitmap( character_bitmaps[ i ][ j ] );
            }
        }
    }

    for( int i = 0; i < 25; ++i )
    {
        al_destroy_bitmap( tiles[ i ] );
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

    quit_game_bounds.x = al_get_display_width( display ) / 2 - al_get_bitmap_width( quit_game_bitmap ) / 2;
    quit_game_bounds.y = al_get_display_height( display ) / 5 * 4 - al_get_bitmap_height( quit_game_bitmap ) / 2;
    quit_game_bounds.width = al_get_bitmap_width( quit_game_bitmap );
    quit_game_bounds.height = al_get_bitmap_height( quit_game_bitmap );

    al_draw_bitmap( quit_game_bitmap,
                    quit_game_bounds.x,
                    quit_game_bounds.y,
                    0 );

}

void render_playing_state()
{
    // Render the background.
    al_clear_to_color( al_map_rgb( 105, 230, 255 ) );

    for( int i = 0; i < block_map->getLength(); ++i )
    {
        for( int j = 0; j < block_map->getHeight(); ++j )
        {
            if( block_map->getBlock( i, j ).getBlockType() )
            {
                al_draw_bitmap( tiles[ block_map->getBlock( i, j ).getBlockType() - 1 ], i * 32, j * 32, 0 );
            }
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

//    if (enemy1 = nullptr){
//        printf("enemy not created");
//        enemy1->type=0;
//        enemy1->current_enemy_bitmap=enemy_bitmaps[enemy1->type][0];
//        enemy1->current_enemy_bound.x=100;
//        enemy1->current_enemy_bound.y = 100;
//        enemy1->current_enemy_bound.width = 43;
//        enemy1->current_enemy_bound.height = 64;
//    }

    // ENEMY 1
    if (enemy1facing%2==0){
        current_enemy_bitmap[0] = enemy_bitmaps[0][enemy1state%4];
    }else{
        current_enemy_bitmap[0] = enemy_bitmaps[0][enemy1state%4+4];
    }

    al_draw_bitmap( current_enemy_bitmap[0], current_enemy_bounds[0].x , current_enemy_bounds[0].y, 0);
    enemy1state++;

    // ENEMY 2
    if (enemy2facing%2==0){
        current_enemy_bitmap[1] = enemy_bitmaps[1][enemy2state%4];
    }else{
        current_enemy_bitmap[1] = enemy_bitmaps[1][enemy2state%4+4];
    }

    al_draw_bitmap( current_enemy_bitmap[1], current_enemy_bounds[1].x , current_enemy_bounds[1].y, 0);
    enemy2state++;

    // ENEMY 3
    if (enemy3facing%2==0){
        current_enemy_bitmap[2] = enemy_bitmaps[2][enemy3state%4];
    }else{
        current_enemy_bitmap[2] = enemy_bitmaps[2][enemy3state%4+4];
    }

    al_draw_bitmap( current_enemy_bitmap[2], current_enemy_bounds[2].x , current_enemy_bounds[2].y, 0);
    enemy3state++;

    // HP bar

    char HPstring[80];
    sprintf(HPstring,"HP: %d",(players[client_id]->get_hitpoint()));

    al_draw_text(default_font, al_map_rgb(221,6,178), 10,10,0,HPstring);
}

void render_pause_state()
{

}

bool check_collision(uint32_t x1, uint32_t y1, uint32_t x2, uint32_t y2) {
    return INTERSECTS( x1, y1, x1+32, y1+64, x2, y2, x2+43, y2+64 );
}

int c_main( int /*argc*/, char **argv )
{

    printf("Starting the application\n");
    Launcher *launcher = new Launcher();

    boost::asio::io_service::work work( client_io_service );
    boost::thread io_thread( boost::bind( &boost::asio::io_service::run, &client_io_service ) );

    client_queue = nullptr;
    display = nullptr;
    timer = nullptr;
    block_map = nullptr;
    bool key[6] = { false, false, false, false, false, false};

    game_state = State_Welcome;
    memset( server_ip_address, '\0', sizeof( server_ip_address ) );

    printf("Setting up local Variables\n");
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
        if( path==nullptr )
        {
            MESSAGE( "CLIENT: Error Getting the path" );
            return -1;
        }
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
        load_images();
        MESSAGE( "CLIENT: Loaded imgaes." );

        al_register_event_source( client_queue, al_get_display_event_source( display ) );
        al_register_event_source( client_queue, al_get_timer_event_source( timer ) );
        al_register_event_source( client_queue, al_get_mouse_event_source() );
        al_register_event_source(client_queue, al_get_keyboard_event_source());

        al_init_user_event_source( &network_event_source );
        al_register_event_source( client_queue, &network_event_source );

        al_start_timer( timer );

        printf("Starting While Loop\n");
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

                    if( key[ KEY_A ] )
                    {
                        if( !players[ client_id ]->get_attacking() && !players[ client_id ]->get_is_hit() )
                        {
                            players[ client_id ]->attack();

                            MESSAGE( "attacking!" );

                            update_obj *newB = new update_obj();

                            newB->id =      bullet_id++;
                            newB->p_id =    client_id;
                            newB->facing =  players[ client_id ]->get_facing();
                            newB->x =       players[ client_id ]->getX() + 16;
                            newB->y =       players[ client_id ]->getY() + 32;
                            newB->type =    BulletType;

                            GameOps.genObject( newB );
                        }
                    }
                    if( key[ KEY_R ] ){
                        players[ client_id ]->set_rebirth();
                    }

                    players[ client_id ]->update();
                    ++counter;

                    BOOST_FOREACH( m_element tmp, objs )
                    {
                        ((GameObject*)(tmp.second))->update();
                    }

                    // ENEMY 1
                    if (current_enemy_bounds[0].x<-37||current_enemy_bounds[0].x>805){
                       enemy1facing++;
                    }
                    if (enemy1facing%2==0){
                        current_enemy_bounds[0].x-=5; // temp in changing enemy location
                    }else{
                        current_enemy_bounds[0].x+=5; // temp in changing enemy location
                    }

                    // ENEMY 2
                    if (current_enemy_bounds[1].x<-37||current_enemy_bounds[1].x>805){
                       enemy2facing++;
                    }
                    if (enemy2facing%2==0){
                        current_enemy_bounds[1].x-=10; // temp in changing enemy location
                    }else{
                        current_enemy_bounds[1].x+=10; // temp in changing enemy location
                    }

                    // ENEMY 3
                    if (current_enemy_bounds[2].x<-37||current_enemy_bounds[2].x>805){
                       enemy3facing++;
                    }
                    if (enemy3facing%2==0){
                        current_enemy_bounds[2].x-=15; // temp in changing enemy location
                    }else{
                        current_enemy_bounds[2].x+=15; // temp in changing enemy location
                    }

                    if ( check_collision( players[ client_id ]->getX(), players[ client_id ]->getY(),
                                     current_enemy_bounds[0].x, current_enemy_bounds[0].y ) )
                    {
                        MESSAGE( "Collided 0" );
                        players[ client_id ]->start_hit();
                        currentHp=players[ client_id ]->get_hitpoint();
                        if(currentHp>0){
                        currentHp--;
                        players[ client_id ]->set_hitpoint(currentHp);
                        }
                        else
                        {
                            players[ client_id ]->set_hitpoint(0);
                            players[ client_id ]->set_is_dead();
                        }


                    }
                    else if ( check_collision( players[ client_id ]->getX(), players[ client_id ]->getY(),
                                     current_enemy_bounds[1].x, current_enemy_bounds[1].y ) )
                    {
                        MESSAGE( "Collided 1" );
                        players[ client_id ]->start_hit();
//                        uint32_t currentHp;
                        currentHp=players[ client_id ]->get_hitpoint();
                        if(currentHp>0){
                        currentHp--;
                        players[ client_id ]->set_hitpoint(currentHp);
                        }
                        else
                        {
                            players[ client_id ]->set_hitpoint(0);

                        }
                    }
                    else if ( check_collision( players[ client_id ]->getX(), players[ client_id ]->getY(),
                                     current_enemy_bounds[2].x, current_enemy_bounds[2].y ) )
                    {
                        MESSAGE( "Collided 2" );
                        players[ client_id ]->start_hit();
//                        uint32_t currentHp;
                        currentHp=players[ client_id ]->get_hitpoint();
                        if(currentHp>0){
                        currentHp--;
                        players[ client_id ]->set_hitpoint(currentHp);
                        }
                        else
                        {
                            players[ client_id ]->set_hitpoint(0);

                        }
                    }
                    else
                    {
                        players[ client_id ]->unhit();
                    }



                    // For now throw a packet out every frame. Not laggy at all.
                    packet_list packet;
                    packet.o_update.id = client_id;
                    packet.o_update.type = PlayerType;
                    packet.o_update.facing = players[ client_id ]->get_facing();
                    packet.o_update.walk_phase = players[ client_id ]->get_walk_phase();
                    packet.o_update.x = (uint32_t)( players[ client_id ]->getX() );
                    packet.o_update.y = (uint32_t)( players[ client_id ]->getY() );

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
                    case ALLEGRO_KEY_R:
                        key[KEY_R] = true;
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

                   case ALLEGRO_KEY_A:
                       key[KEY_A] = false;
                       break;
                   case ALLEGRO_KEY_R:
                       key[KEY_R] = false;
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
