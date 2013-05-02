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
    GameObject::update();

    // do collision detection.
/*
    for( uint32_t col = 0; col < this->m_the_map->getLength(); ++col )
    {
        for( uint32_t row = 0; row < this->m_the_map->getHeight(); ++row )
        {
            if( this->m_the_map->getBlock(col,row).getBlockType() == 1 )
            {
                if( INTERSECTS( m_x, m_y, m_x+m_width, m_y+m_height,
                                col*32, row*32, col*32+32, row*32+32 ) )
                {

                    if( m_x > ( col * 32 ) && m_x < ( col * 32 + 32 ) )
                    {
                        // Moved too far left.
                        m_x = ( col * 32 );
                    }
                    if( ( m_x + m_width ) > ( col * 32 ) && ( m_x + m_width ) < ( col * 32 + 32 ) )
                    {
                        // Moved too far right.
                        m_x = ( col * 32 - 32 );
                    }

                    // Check if hitting something above you
                    if( CONTAINS( m_x, m_y, col*32, row*32, col*32+32, row*32+32 ) ||
                        CONTAINS( m_x+m_width, m_y, col*32, row*32, col*32+32, row*32+32 ) )
                    {
                        m_y = row * 32 + 32;
                        m_vy = 0;
                    }

                    else if ( ( m_y + m_height ) > ( row * 32 ) )
//                    else if( CONTAINS( m_x, m_y+m_height, col*32, row*32, col*32+32, row*32+32 ) ||
//                             CONTAINS( m_x+m_width, m_y+m_height, col*32, row*32, col*32+32, row*32+32 ) )
                    {
                        m_y = ( row * 32 ) - m_height;
                        m_vy = 0;
                        m_is_jumping = false;

                        MESSAGE( "MHH? " );
                    }

                    MESSAGE( "COLLISION" );

                }
            }
        }
    }

            if( this->m_the_map->getBlock( col, row ).getBlockType() == 1 )
            {
                if( m_x > ( col * 32 ) && m_x < ( ( col + 1 ) * 32 ) )
                {
                    if( m_y > ( row * 32 ) && m_y < ( ( row + 1 ) * 32 ) )
                    {
                        MESSAGE( "COLLISION" );

                        m_y = col * 32;
                    }
                }
            }
        }
    }
*/

    if( m_y + m_height > ( 608 ) )
    {
        m_y = 608 - m_height;
        m_vy = 0;
        m_is_jumping = false;
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
