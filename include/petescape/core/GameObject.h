#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <stdint.h>
#include <float.h>
#include "ObjectRenderer.h"
#include "BlockMap.h"

namespace petescape {
namespace core {

class GameObject
{
protected:
    GameObject( uint32_t = 0 );

public:
    inline void setX ( const float &n ){ m_x  = n; }
    inline void setY ( const float &n ){ m_y  = n; }
    inline void setVX( const float &n ){ m_vx = n; }
    inline void setVY( const float &n ){ m_vy = n; }
    inline void setAX( const float &n ){ m_ax = n; }
    inline void setAY( const float &n ){ m_ay = n; }
    inline void setWidth ( const uint32_t &n ){ m_width  = n; }
    inline void setHeight( const uint32_t &n ){ m_height = n; }

    inline const float &getX()  const { return m_x;  }
    inline const float &getY()  const { return m_y;  }
    inline const float &getVX() const { return m_vx; }
    inline const float &getVY() const { return m_vy; }
    inline const float &getAX() const { return m_ax; }
    inline const float &getAY() const { return m_ay; }
    inline const uint32_t &getWidth()  const { return m_width; }
    inline const uint32_t &getHeight() const { return m_height; }

    inline const bool &hasAcceleration() const { return m_use_accel; }
    inline const bool &hasVelocity()     const { return m_use_vel;   }

    inline const uint32_t &getID() const { return m_id; }

    static GameObject* CreateGameObject( );
    static GameObject* CreateGameObject( uint32_t );

    virtual void render();
    virtual void update();
    inline const bool isHeadless(){ return m_renderer != nullptr; }

    inline void setRenderer( ObjectRenderer *obj ){ this->m_renderer = obj; }

    void put_in_map( BlockMap *map  ){ this->m_the_map = map; }

protected:
    uint32_t        m_id;
    ObjectRenderer *m_renderer;

protected:
    // Location members.
    float   m_x;
    float   m_y;

    // Velocity members.
    float   m_vx;
    float   m_vy;

    // Acceleration members. (don't know if we need them)
    float   m_ax;
    float   m_ay;

    // Dimension members.
    uint32_t m_width;
    uint32_t m_height;

    // Variables for determining if the object's
    // location updates by itself
    bool    m_use_accel;
    bool    m_use_vel;

    BlockMap *m_the_map;
};

#define PLAYER_WALK_AMT 8
#define JUMP_VELOCITY -25

#define IS_ZERO( x ) ( x < 0.001 && x > -0.001 )

// top left, bottom right
#define INTERSECTS( x1,y1,x2,y2,x3,y3,x4,y4 ) \
    (!( ((x1)>=(x4) || ((x3)>=(x2) || (y1)>=(y4)) || ((y3)>=(y2))) ))

#define CONTAINS( px,py,x1,y1,x2,y2 ) \
    ( (px>x1) && (px<x2) && (py>y1) && (py<y2) )

class PlayerObject : public GameObject
{
    PlayerObject( uint32_t = 0 );

public:
    static PlayerObject* CreatePlayer();
    static PlayerObject* CreatePlayer( uint32_t );

    void update();

// Movement Stuff
    void start_move_left();
    void start_move_right();
    void stop_moving(){ this->m_vx = 0; }
    void start_hit(){ this->is_hit = 1; }
    void unhit(){ this->is_hit = 0; }
    void start_jump();
    void attack();

// Accessors
    inline uint8_t get_facing(){ return this->m_facing; }
    inline uint8_t get_walk_phase(){ return this->m_walk_phase; }
    inline uint8_t get_attacking(){ return this->is_attacking; }
    inline uint8_t get_is_hit(){ return this->is_hit; }
    inline uint32_t get_hitpoint(){return this->hitpoint;}

// Mutators
    inline void set_facing( uint8_t facing ){ this->m_facing = facing; }
    inline void set_walk_phase( uint8_t walk_phase ){ this->m_walk_phase = walk_phase; }
    inline void set_hitpoint(uint32_t hitpoint){this->hitpoint = hitpoint;}

private:
    /*
     * 0-11 = Walking
     * 12   = Still
     * 13   = Injured
     * 14   = Attacking
     */
    uint8_t m_walk_phase;

    uint8_t is_hit;

    uint8_t is_attacking;
    uint32_t hitpoint;

    /*
     * 0 = Left
     * 1 = Right
     */
    uint8_t m_facing;

    bool m_is_jumping;
};


class Bullet : public GameObject
{
    Bullet( uint32_t = 0, uint8_t = 0, float = 0, float = 0, uint8_t = 0 );

public:
    static Bullet* CreateBullet();
    static Bullet* CreateBullet( uint32_t, uint8_t, float, float, uint8_t );

    void update();

    inline uint8_t      get_pid(){ return this->p_id; }
    inline uint8_t      get_facing(){ return this->m_facing; }

private:
    uint8_t     p_id;
    uint8_t     m_facing;
};


}
}

#endif // GAMEOBJECT_H
