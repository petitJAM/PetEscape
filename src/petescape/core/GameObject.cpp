#include "petescape/core/GameObject.h"

namespace petescape {
namespace core {

GameObject::GameObject(uint32_t id) :
    m_id( id )
{
    this->m_x = 0;
    this->m_y = 0;
    this->m_use_accel = false;
    this->m_use_vel = false;
    this->m_renderer = NULL;
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

}
}
