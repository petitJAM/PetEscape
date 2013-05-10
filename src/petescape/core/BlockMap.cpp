
#include "petescape/core/BlockMap.h"
#include "petescape/core/Block.h"

namespace petescape {
namespace core {

BlockMap::BlockMap(GameMap &map) :
    height( map.getHeight() ),
    length( map.getLength() )
{

    blockmap = (Block**) malloc( sizeof( Block* ) * height * length);

    // copy values in
    for (uint32_t i = 0; i < length; i++)
        for (uint32_t j = 0; j < height; j++)
            addBlockAt(i, j, map.getValue(i, j));
}

BlockMap::~BlockMap()
{
    // fix
    free(blockmap);
}

const uint32_t BlockMap::getHeight()
{
    return height;
}

const uint32_t BlockMap::getLength()
{
    return length;
}

Block BlockMap::getBlock(const uint32_t &x, const uint32_t &y)
{
    return *blockmap[x * height + y];
}

//void BlockMap::setBlock(const uint32_t &x, const uint32_t &y, Block &b)
//{
//    blockmap[x * height + y] = b;
//}

void BlockMap::addBlockAt(const uint32_t &x, const uint32_t &y, const uint16_t &val)
{
    Block* b = new Block(val);
    blockmap[x * height + y] = b;
}

void BlockMap::printMap()
{
    MESSAGE("BLOCKMAP DISPLAY");
    for(uint32_t i = 0; i < getHeight(); i++){
        for(uint32_t j = 0; j < getLength(); j++){
            printf("%d", (int)getBlock(j, i).getBlockType());
        }
        printf("\n");
    }
}

// TODO moving this over
void BlockMap::updateObject( const update_obj *data )
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
//    case EnemyType:
//        if( enemies.count( data->id ) == 0 )
//        {
//            genObject( data );
//        }
//        else
//        {
//            enemies[ data->id ]->setX( data->x );
//            enemies[ data->id ]->setY( data->y );
//            enemies[ data->id ]->set_facing( data->facing );
//            enemies[ data->id ]->set_walk_phase( data->walk_phase );
//        }
//    break;
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

void BlockMap::genObject( const update_obj *data)
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
//    case EnemyType:
//        MESSAGE( "new Enemy Spawning" );
//        obj = EnemyObject::CreateEnemy(data->id );
//        enemies[ data->id ] = static_cast<EnemyObject*>(obj);

//        obj->setRenderer( new petescape::core::EnemyRenderer( enemy_bitmaps[ data->id ] ) );
//        MESSAGE( "new Enemy Spawned" );
//    break;
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

//    obj->put_in_map( block_map ); // TODO
}

void BlockMap::destroyObject( const destroy_obj *data )
{
    switch( data->type )
    {
    case PlayerType:
        players.erase( data->id );
        break;

//    case EnemyType:
//        enemies.erase( data->id );
//        break;

    case OtherType:
        objs.erase( data->id );
        break;

    default:
        MESSAGE( "CLIENT: Invalid object type." );
    }

    objs.erase( data->id );
}

void BlockMap::load_images()
{
    static bool loaded = false;

    if( loaded )
        return;

    // Load character that we need.
    // wtf why did you change the name to cha.bmp... that doesn't even make sense.
    // lol =w = i change it to character happy? :)
    ALLEGRO_BITMAP *char_map = al_load_bitmap( "assets/character/character.bmp" );
    al_convert_mask_to_alpha(char_map,al_map_rgb(255,255,255));

    if (char_map == nullptr){
        MESSAGE( "Could not load character images..." );
        exit( 1 );
    }

    int tile_count = 0;
    ALLEGRO_BITMAP **characters = load_sprite_map( char_map, 32, 64, tile_count );

    if( tile_count != ( 30 * 4 ) )
    {
        MESSAGE( "Didn't load correct image count... " << tile_count );
        exit( 1 );
    }

    if( characters != nullptr )
    {
        for( int i = 0; i < 4; ++i )
        {
            for( int j = 0; j < 30; ++j )
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

    // Load the tileset.
    ALLEGRO_BITMAP *tile_map = al_load_bitmap( "assets/tiles.bmp" );
    tile_count = 0;

    ALLEGRO_BITMAP **temp_tiles = load_sprite_map( tile_map, 32, 32, tile_count );

    for( int i = 0; i < 25; ++i )
        tiles[ i ] = temp_tiles[ i ];

    if( tiles == nullptr )
    {
        MESSAGE( "ERROR LOADING TILES" );
        exit( -2 );
    }

    // load enemies
    ALLEGRO_BITMAP *enemy1_map = al_load_bitmap( "assets/character/enemy_map.bmp" );
    al_convert_mask_to_alpha(enemy1_map,al_map_rgb(221,6,178));

    if (enemy1_map == nullptr){
        MESSAGE( "Could not load enemy1 images..." );
        exit( 1 );
    }

    ALLEGRO_BITMAP **enemies_sprite = load_sprite_map( enemy1_map, 43, 64, tile_count );

    if( tile_count != ( 8 * 3 ) )
    {
        MESSAGE( "Didn't load correct image count... " << tile_count );
        exit( 1 );
    }

    if( enemies_sprite != nullptr )
    {
        for( int i = 0; i < 3; ++i )
        {

            for( int j = 0; j < 8; ++j )
            {
//                enemy_bitmaps[ i ][ j ] = enemies_sprite[ i * 8 + j ];
            }

            // FIXME
//            enemy_char_bitmap[ i ] = characters[ i * 8 ];
//            enemy_char_bounds[ i ].x = 0;
//            enemy_char_bounds[ i ].y = 0;
//            enemy_char_bounds[ i ].width = 43.;
//            enemy_char_bounds[ i ].height = 64;
        }
    }
    else
    {
        MESSAGE( "Could not load character images..." );
        exit( 1 );
    }

    al_destroy_bitmap( enemy1_map );

    loaded = true;
}

void BlockMap::unload_images()
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
}

void BlockMap::render_playing_state()
{
    // Render the background.

    al_clear_to_color( al_map_rgb( 105, 230, 255 ) );

    for( int i = 0; i < length; ++i )
    {
        for( int j = 0; j < height; ++j )
        {
            if( getBlock( i, j ).getBlockType() )
            {
                al_draw_bitmap( tiles[ getBlock( i, j ).getBlockType() - 1 ], i * 32, j * 32, 0 );
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

//    BOOST_FOREACH( m_element tmp, enemies )
//    {
//        ((GameObject*)(tmp.second))->render();
//    }
}

ALLEGRO_BITMAP** BlockMap::load_sprite_map( ALLEGRO_BITMAP *master,
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

void BlockMap::render_pause_state()
{

}

std::map<uint32_t, GameObject *> BlockMap::getGameObjects()
{
    return objs;
}

std::map<uint32_t, PlayerObject *> BlockMap::getPlayers()
{
    return players;
}

}}
