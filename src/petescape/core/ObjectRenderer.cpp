#include "petescape/core/ObjectRenderer.h"
#include "petescape/core/GameObject.h"

#include "allegro5/allegro.h"

namespace petescape {
namespace core {

void PoorRenderer::render( GameObject* obj )
{
    for( uint32_t i = 0; i < obj->getWidth(); ++i )
        for( uint32_t j = 0; j < obj->getHeigth(); ++j )
            al_draw_pixel( obj->getX() + i, obj->getY() + j, al_map_rgb( 0, 0, 0 ) );
}

}
}
