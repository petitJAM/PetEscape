#include "petescape/core/GameObject.h"
#include <cstdlib>

namespace petescape {
namespace core {

GameObject::GameObject(uint32_t id) :
    m_id( id )
{
    this->m_x = 0;
    this->m_y = 0;
    this->m_width = 10;
    this->m_height = 10;
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
    this->m_renderer->render( this );
}

PlayerObject::PlayerObject( uint32_t id ) :
    GameObject( id )
{
    // Any Player specific values get set here
}

PlayerObject* PlayerObject::CreatePlayer()
{
    static uint32_t n_id = 0;

    return CreatePlayer( n_id );
}

PlayerObject* PlayerObject::CreatePlayer( uint32_t id )
{
    PlayerObject *obj = new PlayerObject( id );

    // Initialize any player data.

    return obj;
}

}
}
