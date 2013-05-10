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
   KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, KEY_A
};

// cut stuff

#endif // CLIENT_RESOURCES_H
