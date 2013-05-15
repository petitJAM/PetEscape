#include "petescape/core/GameObject.h"
#include "petescape/core/core_defs.h"
#include "petescape/core/BlockMap.h"
#include "petescape/core/Block.h"
#include <cstdlib>

namespace petescape {
namespace core {

GameObject::GameObject( uint32_t id ) :
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
    this->m_height = 60;                                        // FIXME
    this->m_is_jumping = false;
    this->hitpoint = 100;
    this->is_attacking = 0;
    this->is_dead = false;

    this->m_x = 400;
    this->m_y = 480;

    this->m_type = PlayerType;
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
    if(!is_dead && !this->m_is_jumping && !this->is_hit && !this->is_attacking)
    {
        this->m_vy = JUMP_VELOCITY;
        this->m_walk_phase = 3;
        this->m_is_jumping = true;
    }
//    else if (!this->is_attacking && this->m_is_jumping)
//    {
//        this->m_vy = JUMP_VELOCITY / 4;
//        this->m_walk_phase = 3;
//        this->m_is_jumping = true;
//    }
}

void PlayerObject::attack(){
    if (this->is_attacking == 0)
    {
        this->is_attacking = 4;
    }
}

void PlayerObject::update()
{
    int h13 = 15; // height / 3
    int h23 = h13 * 2; // 2*height / 3
    // do collision detection.
    float new_x1 = m_x + m_vx + m_ax;
    float new_x2 = m_x + m_vx + m_ax + m_width / 2;
    float new_x3 = m_x + m_vx + m_ax + m_width;
    float new_y1 = m_y + m_vy + m_ay;
    float new_y2 = m_y + m_vy + m_ay + h13; // ~ height / 3
    float new_y3 = m_y + m_vy + m_ay + h23; // ~ 2*height / 3
    float new_y4 = m_y + m_vy + m_ay + m_height;

    bool x_collision = true;
    bool y_collision = true;

    for( uint32_t i = 0; i < m_the_map->getLength(); ++i )
    {
        for( uint32_t j = 0; j < m_the_map->getHeight(); ++j )
        {
            if( m_the_map->getBlock( i, j ).getBlockType() != 0 )
            {
                if( CONTAINS( new_x1, m_y            , i*32, j*32, i*32+32, j*32+32 ) ||
                    CONTAINS( new_x1, m_y + h13      , i*32, j*32, i*32+32, j*32+32 ) ||
                    CONTAINS( new_x1, m_y + h23      , i*32, j*32, i*32+32, j*32+32 ) ||
                    CONTAINS( new_x1, m_y + m_height, i*32, j*32, i*32+32, j*32+32 ) )
                {
                    m_x = i * 32 + 32;
                    goto end_col_check_x;
                }

                if( CONTAINS( new_x2, m_y            , i*32, j*32, i*32+32, j*32+32 ) ||
                    CONTAINS( new_x2, m_y + h13      , i*32, j*32, i*32+32, j*32+32 ) ||
                    CONTAINS( new_x2, m_y + h23      , i*32, j*32, i*32+32, j*32+32 ) ||
                    CONTAINS( new_x2, m_y + m_height, i*32, j*32, i*32+32, j*32+32 ) )
                {
                    m_x = i * 32 + 16;
                    goto end_col_check_x;
                }

                if( CONTAINS( new_x3, m_y            , i*32, j*32, i*32+32, j*32+32 ) ||
                    CONTAINS( new_x3, m_y + h13      , i*32, j*32, i*32+32, j*32+32 ) ||
                    CONTAINS( new_x3, m_y + h23      , i*32, j*32, i*32+32, j*32+32 ) ||
                    CONTAINS( new_x3, m_y + m_height, i*32, j*32, i*32+32, j*32+32 ) )
                {
                    m_x = i * 32 - 32;
                    goto end_col_check_x;
                }
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
                if( CONTAINS( m_x            , new_y4, i*32, j*32, i*32+32, j*32+32 ) ||
                    CONTAINS( m_x + m_width/2, new_y4, i*32, j*32, i*32+32, j*32+32 ) ||
                    CONTAINS( m_x + m_width  , new_y4, i*32, j*32, i*32+32, j*32+32 ) )
                {
                    m_y = j * 32 - m_height;
                    m_is_jumping = false;
                    goto end_col_check_y;
                }

                if( CONTAINS( m_x            , new_y3, i*32, j*32, i*32+32, j*32+32 ) ||
                    CONTAINS( m_x + m_width/2, new_y3, i*32, j*32, i*32+32, j*32+32 ) ||
                    CONTAINS( m_x + m_width  , new_y3, i*32, j*32, i*32+32, j*32+32 ) )
                {
                    goto end_col_check_y;
                }

                if( CONTAINS( m_x            , new_y2, i*32, j*32, i*32+32, j*32+32 ) ||
                    CONTAINS( m_x + m_width/2, new_y2, i*32, j*32, i*32+32, j*32+32 ) ||
                    CONTAINS( m_x + m_width  , new_y2, i*32, j*32, i*32+32, j*32+32 ) )
                {
                    goto end_col_check_y;
                }

                // Top of character colides with something.
                if( CONTAINS( m_x            , new_y1, i*32, j*32, i*32+32, j*32+32 ) ||
                    CONTAINS( m_x + m_width/2, new_y1, i*32, j*32, i*32+32, j*32+32 ) ||
                    CONTAINS( m_x + m_width  , new_y1, i*32, j*32, i*32+32, j*32+32 ) )
                {
                    // Assume that we're moving up.
                    m_y = j * 32 + 32;
                    goto end_col_check_y;
                }
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

    if (!this->is_dead)
    {

        if (this->is_hit)
            this->m_walk_phase = 13;
        else if( this->m_is_jumping )
            this->m_walk_phase = 1;
        else if( !IS_ZERO( this->m_vx ) )
            this->m_walk_phase = ( this->m_walk_phase + 1 ) % 12;
        else
            this->m_walk_phase = 12;

        if (this->is_attacking)
        {
            this->m_walk_phase=14;
            this->is_attacking--;
            printf("is_attacking %d\n", is_attacking);
        }
    }
    else
    {
        this->m_walk_phase=15;
        if (this->is_attacking)
            this->is_attacking--;
    }

    GLOBAL_RENDER_OFFSET += -this->m_vx;
}

void PlayerObject::start_move_left(){
    if (!is_dead&&!is_hit && !is_attacking)
    {
        this->m_vx = -PLAYER_WALK_AMT;
        this->m_facing = 0;
    }
    else
    {
        this->m_vx = 0;
    }
}

void PlayerObject::start_move_right(){
    if (!is_dead&&!is_hit && !is_attacking)
    {
        this->m_vx = PLAYER_WALK_AMT;
        this->m_facing = 1;
    }
    else
    {
        this->m_vx = 0;
    }
}


EnemyObject::EnemyObject( uint32_t id, float x, float y, uint16_t type) :
    GameObject( id )
{
    // Any Enemy specific values get set here
    this->m_use_accel = true;
    this->m_use_vel = true;
    this->m_ay = 2;
    this->m_facing = 0;
    this->m_walk_phase = 0;
    this->m_width = 43;
    this->m_height = 64; // FIXME
    this->m_is_jumping = false;
    this->is_attacking = 0;

    this->m_vx = 5;
    this->m_vy = 0;

    this->m_x = x;
    this->m_y = y;

    this->m_type = EnemyType;
    this->e_type = type;
}

EnemyObject* EnemyObject::CreateEnemy()
{
    static uint32_t e_id = 0;

    srand( time( nullptr ) );
    int temp = rand();
    int spawnPoint = (temp % (32 * (MAP_LENGTH - 2))) + 32;
    return CreateEnemy( e_id++, spawnPoint, 480, temp % 3);

}

EnemyObject* EnemyObject::CreateEnemy( uint32_t id, float x, float y, uint16_t type)
{
    EnemyObject *e = new EnemyObject( id, x, y, type );

    return e;
}

void EnemyObject::update(){
    //std::cerr << this->getID() << " " << this->getX() << " " << this->getY() << " " << this->get_enemy_type() << std::endl;
    int speed_mult = (e_type % 3) + 1;

    int h13 = m_height / 3; // height / 3
    int h23 = h13 * 2; // 2*height / 3
    // do collision detection.
    float new_x1 = m_x + m_vx + m_ax;
    float new_x2 = m_x + m_vx + m_ax + m_width / 2;
    float new_x3 = m_x + m_vx + m_ax + m_width;

    bool x_collision = true;

    for( uint32_t i = 0; i < m_the_map->getLength(); ++i )
    {
        for( uint32_t j = 0; j < m_the_map->getHeight(); ++j )
        {
            if( m_the_map->getBlock( i, j ).getBlockType() != 0 )
            {
                if( CONTAINS( new_x1, m_y            , i*32, j*32, i*32+32, j*32+32 ) ||
                    CONTAINS( new_x1, m_y + h13      , i*32, j*32, i*32+32, j*32+32 ) ||
                    CONTAINS( new_x1, m_y + h23      , i*32, j*32, i*32+32, j*32+32 ) ||
                    CONTAINS( new_x1, m_y + m_height , i*32, j*32, i*32+32, j*32+32 ) )
                {
                    m_x = i * 32 + m_width;
                    goto end_col_check_x_for_enemies;
                }

                if( CONTAINS( new_x2, m_y            , i*32, j*32, i*32+32, j*32+32 ) ||
                    CONTAINS( new_x2, m_y + h13      , i*32, j*32, i*32+32, j*32+32 ) ||
                    CONTAINS( new_x2, m_y + h23      , i*32, j*32, i*32+32, j*32+32 ) ||
                    CONTAINS( new_x2, m_y + m_height , i*32, j*32, i*32+32, j*32+32 ) )
                {
                    m_x = i * 32 + m_width / 2 + 1;
                    goto end_col_check_x_for_enemies;
                }

                if( CONTAINS( new_x3, m_y            , i*32, j*32, i*32+32, j*32+32 ) ||
                    CONTAINS( new_x3, m_y + h13      , i*32, j*32, i*32+32, j*32+32 ) ||
                    CONTAINS( new_x3, m_y + h23      , i*32, j*32, i*32+32, j*32+32 ) ||
                    CONTAINS( new_x3, m_y + m_height , i*32, j*32, i*32+32, j*32+32 ) )
                {
                    m_x = i * 32 - m_width;
                    goto end_col_check_x_for_enemies;
                }
            }
        }
    }
    x_collision = false;
end_col_check_x_for_enemies:

    if(x_collision)
        m_facing = !m_facing;

    //check if it's dead here
    if(true){
        if(!m_facing){
            m_x -= m_vx * speed_mult;
            m_walk_phase = (m_walk_phase + 1) % 3;
        }
        else{
            m_x += m_vx * speed_mult;
            m_walk_phase = (m_walk_phase + 1) % 3;
        }
    }
}

Bullet::Bullet( uint32_t id, uint8_t p_id, float x, float y, uint8_t facing ) :
    GameObject( id )
{
    // Any Bullet specific values get set here
    this->m_use_vel = true;
    this->m_facing = facing;
    this->m_width = 7;
    this->m_height = 2;     //FIXME
    this->p_id = p_id;

    this->m_vx = 16;
    this->m_vy = 0;

    this->m_x = x;
    this->m_y = y;

    this->m_type = BulletType;
}

Bullet* Bullet::CreateBullet()
{
    static uint32_t n_id = 0;

    return CreateBullet( n_id++, 0, 10, 10, 0 );
}

Bullet* Bullet::CreateBullet( uint32_t id, uint8_t p_id, float x, float y, uint8_t facing )
{
    Bullet *b = new Bullet( id, p_id, x, y, facing );

    return b;
}

void Bullet::update()
{
    if( this->m_facing )
    {
        this->m_x += this->m_vx;
        this->m_y += this->m_vy;
    }
    else
    {
        this->m_x -= this->m_vx;
        this->m_y -= this->m_vy;
    }
}



}
}
