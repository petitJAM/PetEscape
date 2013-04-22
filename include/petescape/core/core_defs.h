#ifndef CORE_DEFS_H
#define CORE_DEFS_H

#define MAP_LENGTH 100
#define MAP_HEIGHT 40

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
