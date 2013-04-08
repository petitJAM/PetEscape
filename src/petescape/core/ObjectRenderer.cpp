#include "petescape/core/ObjectRenderer.h"
#include "petescape/core/GameObject.h"

#include "allegro5/allegro.h"

namespace petescape {
namespace core {

void PoorRenderer::render( GameObject* obj )
{
    al_draw_pixel( obj->getX(), obj->getY(), al_map_rgb( 0, 0, 0 ) );
}

}
}
