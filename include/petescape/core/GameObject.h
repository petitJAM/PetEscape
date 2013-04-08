#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <stdint.h>
#include <float.h>
#include "ObjectRenderer.h"

namespace petescape {
namespace core {

class GameObject
{
    GameObject( uint32_t = 0 );

public:
    inline void setX ( const float &n ){ m_x  = n; }
    inline void setY ( const float &n ){ m_y  = n; }
    inline void setVX( const float &n ){ m_vx = n; }
    inline void setVY( const float &n ){ m_vy = n; }
    inline void setAX( const float &n ){ m_ax = n; }
    inline void setAY( const float &n ){ m_ay = n; }

    inline const float &getX() { return m_x;  }
    inline const float &getY() { return m_y;  }
    inline const float &getVX(){ return m_vx; }
    inline const float &getVY(){ return m_vy; }
    inline const float &getAX(){ return m_ax; }
    inline const float &getAY(){ return m_ay; }

    inline const bool &hasAcceleration(){ return m_use_accel; }
    inline const bool &hasVelocity()    { return m_use_vel;   }

    inline const uint32_t &getID(){ return m_id; }

    static GameObject* CreateGameObject( );
    static GameObject* CreateGameObject( uint32_t );

    virtual void render();
    inline const bool isHeadless(){ return m_renderer != nullptr; }

    inline void setRenderer( ObjectRenderer *obj ){ this->m_renderer = obj; }

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

    // Variables for determining if the object's
    // location updates by itself
    bool    m_use_accel;
    bool    m_use_vel;
};

}
}

#endif // GAMEOBJECT_H
