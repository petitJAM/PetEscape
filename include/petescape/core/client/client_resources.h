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

//namespace petescape {
//namespace core {
//namespace client {

const float FPS = 60;

enum MYKEYS {
   KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT
};


using namespace petescape::core;

typedef std::pair< uint32_t, GameObject* > m_element;

ALLEGRO_BITMAP *play_solo_bitmap,
               *host_game_bitmap,
               *join_game_bitmap,
               *quit_game_bitmap,
               *character_bitmap;

Rectange play_solo_bounds,
         host_game_bounds,
         join_game_bounds,
         quit_game_bounds,
         character_bounds;


ALLEGRO_FONT *default_font;

//ALLEGRO_DISPLAY *display;


//ALLEGRO_EVENT_QUEUE *client_queue;

//ALLEGRO_EVENT_SOURCE network_event_source;

//ALLEGRO_TIMER *timer;
//boost::asio::io_service client_io_service;
//boost::asio::ip::tcp::socket *socket;

//petescape::networking::common::network_packet input;

//std::map<uint32_t, petescape::core::GameObject *>   objs;
//std::map<uint32_t, petescape::core::PlayerObject *> players;

//uint8_t    client_id;
//uint8_t    map_length;
//uint8_t    map_height;

//petescape::core::GameMap   *map;

//GameState  game_state;

//int       num_map_packets_recieved;

//char server_ip_address[ 20 ];
//bool accepting_ip_address;

void render_welcome_state();
void render_playing_state();
void render_paused_state();

//}}}

#endif // CLIENT_RESOURCES_H
