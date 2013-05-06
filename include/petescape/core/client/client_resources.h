#ifndef CLIENT_RESOURCES_H
#define CLIENT_RESOURCES_H

#pragma once

#include <boost/asio.hpp>
#include <allegro5/allegro.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_font.h>

#include "petescape/core/core_defs.h"
#include "petescape/core/GameMap.h"
#include "petescape/core/GameObject.h"
#include "petescape/networking/common/net_struct.h"

const float FPS = 15;
const float MovementSpeed = 5.00;
const float JumpInitVelocity = 60.00;

enum MYKEYS {
   KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT
};

typedef struct
{
    ALLEGRO_BITMAP *character_bitmaps[4][30];
    ALLEGRO_BITMAP *current_char_bitmap[4];
    Rectange current_char_bounds[4];
    int32_t health;
    int32_t damage;
    bool is_dead;
    int32_t dx;
    int32_t dy;
} Character;

using namespace petescape::core;

typedef std::pair< uint32_t, GameObject* > m_element;

ALLEGRO_BITMAP *play_solo_bitmap,
               *host_game_bitmap,
               *join_game_bitmap,
               *quit_game_bitmap,
               *character_bitmaps[4][30],
               *current_char_bitmap[4];

ALLEGRO_BITMAP *tiles[25];

Rectange play_solo_bounds,
         host_game_bounds,
         join_game_bounds,
         quit_game_bounds,
         current_char_bounds[4];

Character current_player;


ALLEGRO_FONT *default_font;

#endif // CLIENT_RESOURCES_H
