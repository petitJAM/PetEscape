#ifndef CORE_DEFS_H
#define CORE_DEFS_H

#define MAP_LENGTH 100
#define MAP_HEIGHT 40

#define KEY_UP_INDEX 0
#define KEY_DOWN_INDEX 1
#define KEY_LEFT_INDEX 2
#define KEY_RIGHT_INDEX 3
#define KEY_SPACE_INDEX 4
#define TOTAL_SUPPORTED_KEYS 5

#define PLAYER_SPEED 8

#define MAX_CONNECTIONS 4

#include <iostream>

#ifdef DEBUG
 #define MESSAGE( x ) do{ \
     std::cerr << x << std::endl; \
     } while( 0 )
#else
 #define MESSAGE( x )
#endif

enum GameObjectType
{
    BlockType = 0x00,
    PlayerType,
    OtherType,

    // Add new types above this.
    TypeCount
};

#endif // CORE_DEFS_H
