#ifndef BLOCKMAP_H
#define BLOCKMAP_H

#include "petescape/core/GameObject.h"
#include "petescape/core/Block.h"
#include "petescape/core/GameMap.h"

#include <allegro5/allegro_font.h>

#include <boost/foreach.hpp>

#include <map>
#include <cstdlib>

namespace petescape {
namespace core {

class BlockMap
{
public:
    BlockMap( GameMap &map );

    ~BlockMap();

    inline const uint32_t getHeight();
    inline const uint32_t getLength();
    Block getBlock( const uint32_t &x, const uint32_t &y );
    inline void setBlock( const uint32_t &x, const uint32_t &y, Block &b);

    void addBlockAt( const uint32_t &x, const uint32_t &y, const uint16_t &val);

    void printMap();

    void updateObject( const update_obj *data );
    void genObject( const update_obj *data);
    void destroyObject( const destroy_obj *data );

    void BlockMap::load_images();
    void BlockMap::unload_images();
    void BlockMap::render_welcome_state();
    void BlockMap::render_playing_state();
    ALLEGRO_BITMAP** BlockMap::load_sprite_map( ALLEGRO_BITMAP *master,
                                      int32_t t_w,
                                      int32_t t_h,
                                      int32_t &tile_count );
    void BlockMap::render_pause_state();

    std::map<uint32_t, GameObject *> getGameObjects();
    std::map<uint32_t, PlayerObject *> getPlayers();

private:
    // std::map<uint32_t, EnemyObject *>   enemies;    // remove later
    std::map<uint32_t, GameObject *>    objs;
    std::map<uint32_t, PlayerObject *>  players;

    uint32_t    height;
    uint32_t    length;
    Block     **blockmap;
};

}}

#endif // BLOCKMAP_H

using namespace petescape::core;

typedef std::pair< uint32_t, GameObject* > m_element;

ALLEGRO_BITMAP *play_solo_bitmap,
               *host_game_bitmap,
               *join_game_bitmap,
               *quit_game_bitmap,
               *character_bitmaps[4][30],
               *current_char_bitmap[4];

ALLEGRO_BITMAP  *tiles[25];
ALLEGRO_DISPLAY *display;

Rectange play_solo_bounds,
          host_game_bounds,
          join_game_bounds,
          quit_game_bounds,
          current_char_bounds[4];

ALLEGRO_FONT *default_font;
