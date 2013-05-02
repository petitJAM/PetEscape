#include "petescape/core/GameObject.h"
#include "petescape/core/core_defs.h"
#include "petescape/core/BlockMap.h"
#include "petescape/core/Block.h"
#include <cstdlib>

namespace petescape {
namespace core {

GameObject::GameObject(uint32_t id) :
    m_id( id )
{
    this->m_x = 0;
    this->m_y = 0;
    this->m_vx = 0;
    this->m_vy = 0;
    this->m_ax = 0;
    this->m_ay = 0;
    this->m_width = 0;
    this->m_height = 0;
    this->m_use_accel = false;
    this->m_use_vel = false;
    this->m_renderer = nullptr;
}

GameObject* GameObject::CreateGameObject()
{
    static uint32_t n_id = 0;

    return CreateGameObject( n_id++ );
}

GameObject* GameObject::CreateGameObject( uint32_t id )
{
    GameObject *obj = new GameObject( id );

    // We can do other stuff here, like set values that
    // may be passed in with the id.

    return obj;
}

void GameObject::render()
{
    if( this->m_renderer )
        this->m_renderer->render( this );
}

void GameObject::update()
{
    if( this->m_use_accel )
    {
        this->m_vx += this->m_ax;
        this->m_vy += this->m_ay;
    }

    if( this->m_use_vel )
    {
        this->m_x += this->m_vx;
        this->m_y += this->m_vy;
    }
}

PlayerObject::PlayerObject( uint32_t id ) :
    GameObject( id )
{
    // Any Player specific values get set here
    this->m_use_accel = true;
    this->m_use_vel = true;
    this->m_ay = 2;
    this->m_facing = 0;
    this->m_walk_phase = 0;
    this->m_width = 32;
    this->m_height = 64;
    this->m_is_jumping = false;

    this->m_x = 64;
    this->m_y = 128;
}

PlayerObject* PlayerObject::CreatePlayer()
{
    static uint32_t n_id = 0;

    return CreatePlayer( n_id++ );
}

PlayerObject* PlayerObject::CreatePlayer( uint32_t id )
{
    PlayerObject *obj = new PlayerObject( id );

    return obj;
}

void PlayerObject::start_jump()
{
    if( !this->m_is_jumping )
    {
        this->m_vy = JUMP_VELOCITY;
        this->m_walk_phase = 3;
        this->m_is_jumping = true;
    }
}

void PlayerObject::update()
{
    // do collision detection.
    float new_x1 = m_x + m_vx + m_ax;
    float new_x2 = m_x + m_vx + m_ax + m_width / 2;
    float new_x3 = m_x + m_vx + m_ax + m_width;
    float new_y1 = m_y + m_vy + m_ay;
    float new_y2 = m_y + m_vy + m_ay + 21; // ~ height / 3
    float new_y3 = m_y + m_vy + m_ay + 42; // ~ 2*height / 3
    float new_y4 = m_y + m_vy + m_ay + m_height;

    bool x_collision = true;
    bool y_collision = true;

    for( uint32_t i = 0; i < m_the_map->getLength(); ++i )
    {
        for( uint32_t j = 0; j < m_the_map->getHeight(); ++j )
        {
            if( m_the_map->getBlock( i, j ).getBlockType() != 0 )
            {
                if( CONTAINS( new_x1, m_y           , i*32, j*32, i*32+32, j*32+32 ) ) goto end_col_check_x;
                if( CONTAINS( new_x1, m_y + 21      , i*32, j*32, i*32+32, j*32+32 ) ) goto end_col_check_x;
                if( CONTAINS( new_x1, m_y + 42      , i*32, j*32, i*32+32, j*32+32 ) ) goto end_col_check_x;
                if( CONTAINS( new_x1, m_y + m_height, i*32, j*32, i*32+32, j*32+32 ) ) goto end_col_check_x;
                if( CONTAINS( new_x2, m_y           , i*32, j*32, i*32+32, j*32+32 ) ) goto end_col_check_x;
                if( CONTAINS( new_x2, m_y + 21      , i*32, j*32, i*32+32, j*32+32 ) ) goto end_col_check_x;
                if( CONTAINS( new_x2, m_y + 42      , i*32, j*32, i*32+32, j*32+32 ) ) goto end_col_check_x;
                if( CONTAINS( new_x2, m_y + m_height, i*32, j*32, i*32+32, j*32+32 ) ) goto end_col_check_x;
                if( CONTAINS( new_x3, m_y           , i*32, j*32, i*32+32, j*32+32 ) ) goto end_col_check_x;
                if( CONTAINS( new_x3, m_y + 21      , i*32, j*32, i*32+32, j*32+32 ) ) goto end_col_check_x;
                if( CONTAINS( new_x3, m_y + 42      , i*32, j*32, i*32+32, j*32+32 ) ) goto end_col_check_x;
                if( CONTAINS( new_x3, m_y + m_height, i*32, j*32, i*32+32, j*32+32 ) ) goto end_col_check_x;
            }
        }
    }
    x_collision = false;
end_col_check_x:

    for( uint32_t i = 0; i < m_the_map->getLength(); ++i )
    {
        for( uint32_t j = 0; j < m_the_map->getHeight(); ++j )
        {
            if( m_the_map->getBlock( i, j ).getBlockType() != 0 )
            {
                if( CONTAINS( m_x            , new_y1, i*32, j*32, i*32+32, j*32+32 ) ) goto end_col_check_y;
                if( CONTAINS( m_x            , new_y2, i*32, j*32, i*32+32, j*32+32 ) ) goto end_col_check_y;
                if( CONTAINS( m_x            , new_y3, i*32, j*32, i*32+32, j*32+32 ) ) goto end_col_check_y;
                if( CONTAINS( m_x            , new_y4, i*32, j*32, i*32+32, j*32+32 ) ) { m_is_jumping = false; goto end_col_check_y; }
                if( CONTAINS( m_x + m_width/2, new_y1, i*32, j*32, i*32+32, j*32+32 ) ) goto end_col_check_y;
                if( CONTAINS( m_x + m_width/2, new_y2, i*32, j*32, i*32+32, j*32+32 ) ) goto end_col_check_y;
                if( CONTAINS( m_x + m_width/2, new_y3, i*32, j*32, i*32+32, j*32+32 ) ) goto end_col_check_y;
                if( CONTAINS( m_x + m_width/2, new_y4, i*32, j*32, i*32+32, j*32+32 ) ) { m_is_jumping = false; goto end_col_check_y; }
                if( CONTAINS( m_x + m_width  , new_y1, i*32, j*32, i*32+32, j*32+32 ) ) goto end_col_check_y;
                if( CONTAINS( m_x + m_width  , new_y2, i*32, j*32, i*32+32, j*32+32 ) ) goto end_col_check_y;
                if( CONTAINS( m_x + m_width  , new_y3, i*32, j*32, i*32+32, j*32+32 ) ) goto end_col_check_y;
                if( CONTAINS( m_x + m_width  , new_y4, i*32, j*32, i*32+32, j*32+32 ) ) { m_is_jumping = false; goto end_col_check_y; }
            }
        }
    }
    y_collision = false;
end_col_check_y:

    if( x_collision )
    {
//        MESSAGE( "X COLLISION" );
        m_vx = 0;
    }
    else
    {
        m_vx += m_ax;
        m_x += m_vx;
    }

    if( y_collision )
    {
//        MESSAGE( "Y COLLISION" );
        m_vy = 0;
    }
    else
    {
        m_vy += m_ay;
        m_y += m_vy;
    }

    if( this->m_is_jumping )
        this->m_walk_phase = 3;
    else if( !IS_ZERO( this->m_vx ) )
        this->m_walk_phase = ( this->m_walk_phase % 2 ) + 1;
    else
        this->m_walk_phase = 0;
}

}
}
