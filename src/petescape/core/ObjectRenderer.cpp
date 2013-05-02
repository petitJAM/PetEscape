#include "petescape/core/ObjectRenderer.h"
#include "petescape/core/GameObject.h"
#include "petescape/core/core_defs.h"

#include <allegro5/allegro.h>

namespace petescape {
namespace core {

void PoorRenderer::render( GameObject* obj )
{
    for( uint32_t i = 0; i < obj->getWidth(); ++i )
        for( uint32_t j = 0; j < obj->getHeigth(); ++j )
            al_draw_pixel( obj->getX() + i,
                           obj->getY() + j,
                           al_map_rgb( 0, 0, 0 ) );
}

PlayerRenderer::PlayerRenderer( ALLEGRO_BITMAP *images[] )
{
    for( int i = 0; i < 8; ++i )
        this->m_sprites[ i ] = images[ i ];
}

void PlayerRenderer::render( GameObject* obj )
{
    PlayerObject *player = static_cast<PlayerObject*>( obj );

    if( player != nullptr )
    {
        uint8_t index = player->get_walk_phase();
        index += player->get_facing() ? 4 : 0;

        if( m_sprites[ index ] == nullptr )
        {
            MESSAGE( "Error accessing player sprite." );
            return;
        }

        al_draw_bitmap( this->m_sprites[ index ],
                        player->getX(),
                        player->getY(),
                        0 );
    }
}

}}
