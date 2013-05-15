#ifndef CORE_DEFS_H
#define CORE_DEFS_H

//#define MAP_LENGTH 100
//#define MAP_HEIGHT 40
#define MAP_LENGTH 200
#define MAP_HEIGHT 19

#define KEY_UP_INDEX 0
#define KEY_DOWN_INDEX 1
#define KEY_LEFT_INDEX 2
#define KEY_RIGHT_INDEX 3
#define KEY_SPACE_INDEX 4
#define TOTAL_SUPPORTED_KEYS 5

#define PLAYER_SPEED 8

#define MAX_CONNECTIONS 4

extern float GLOBAL_RENDER_OFFSET;

#include <iostream>
#include <stdint.h>

#ifdef DEBUG
 #define MESSAGE( x ) do{ \
     std::cerr << x << std::endl; \
     } while( 0 )
#else
 #define MESSAGE( x )
#endif

typedef struct
{
    int32_t x;
    int32_t y;
    int32_t width;
    int32_t height;
} Rectange;

#define IS_WITHIN(rect__, x__, y__) \
    ((x__>=rect__.x)&&(x__<=(rect__.x+rect__.width)) && \
    (y__>=rect__.y)&&(y__<=(rect__.y+rect__.height)))

enum GameState
{
    WelcomeState,
    SetupState,
    PlayingState,
    PausedState
};

enum NewGameState
{
    State_Welcome,
    State_Init,
    State_LoadMap,
    State_LoadObjects,
    State_Playing
};

enum GameObjectType
{
    BlockType,
    PlayerType,
    EnemyType,
    BulletType,
    OtherType,

    // Add new types above this.
    TypeCount
};

#endif // CORE_DEFS_H
