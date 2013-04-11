#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <stdint.h>
#include <float.h>
#include "ObjectRenderer.h"

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
    inline const uint32_t &getHeigth() const { return m_height; }

    inline const bool &hasAcceleration() const { return m_use_accel; }
    inline const bool &hasVelocity()     const { return m_use_vel;   }

    inline const uint32_t &getID() const { return m_id; }

    static GameObject* CreateGameObject( );
    static GameObject* CreateGameObject( uint32_t );

    virtual void render();
    inline const bool isHeadless(){ return m_renderer != nullptr; }

    inline void setRenderer( ObjectRenderer *obj ){ this->m_renderer = obj; }

    bool willCollideWith( GameObject const * ) const;

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
};

class PlayerObject : public GameObject
{
    PlayerObject( uint32_t = 0 );

public:
    static PlayerObject* CreatePlayer();
    static PlayerObject* CreatePlayer( uint32_t );
};

}
}

#endif // GAMEOBJECT_H
