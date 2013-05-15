#include "petescape/core/ObjectRenderer.h"
#include "petescape/core/GameObject.h"
#include "petescape/core/core_defs.h"

#include <allegro5/allegro.h>

namespace petescape {
namespace core {

void PoorRenderer::render( GameObject* obj )
{
    for( uint32_t i = 0; i < obj->getWidth(); ++i )
        for( uint32_t j = 0; j < obj->getHeight(); ++j )
            al_draw_pixel( obj->getX() + i + GLOBAL_RENDER_OFFSET,
                           obj->getY() + j,
                           al_map_rgb( 0, 0, 0 ) );
}

PlayerRenderer::PlayerRenderer( ALLEGRO_BITMAP *images[] )
{
    for( int i = 0; i < 30; ++i )
        this->m_sprites[ i ] = images[ i ];
}

void PlayerRenderer::render( GameObject* obj )
{
    PlayerObject *player = static_cast<PlayerObject*>( obj );

    if( player != nullptr )
    {
        uint8_t index = 0;
        uint8_t phase = player->get_walk_phase();

        if( player->get_facing() == 1 )
        {
            if( phase == 12 )
                index = 0;
            else if( phase == 13 )
                index = 2;
            else if( phase == 14 )
                index = 4;
            else
                index = 6 + phase;
        }
        else
        {
            if( phase == 12 )
                index = 1;
            else if( phase == 13 )
                index = 3;
            else if( phase == 14 )
                index = 5;
            else
                index = 29 - phase;
        }

        if( m_sprites[ index ] == nullptr )
        {
            MESSAGE( "Error accessing player sprite." );
            return;
        }

        al_draw_bitmap( this->m_sprites[ index ],
                        player->getX() + GLOBAL_RENDER_OFFSET,
                        player->getY(),
                        0 );
    }
}

void EnemyRenderer::render( GameObject* obj )
{
    EnemyObject *enemy = static_cast<EnemyObject*>( obj );

    if( enemy != nullptr )
    {
        uint8_t index = 0;
        uint8_t phase = enemy->get_walk_phase();

        if( enemy->get_facing() == 1 )
        {
            // TODO FIXME
//            if( phase == 12 )
//                index = 0;
//            else if( phase == 13 )
//                index = 2;
//            else if( phase == 14 )
//                index = 4;
//            else
//                index = 6 + phase;
        }
        else
        {
//            if( phase == 12 )
//                index = 1;
//            else if( phase == 13 )
//                index = 3;
//            else if( phase == 14 )
//                index = 5;
//            else
//                index = 29 - phase;
        }

        if( m_sprites[ index ] == nullptr )
        {
            MESSAGE( "Error accessing player sprite." );
            return;
        }

        al_draw_bitmap( this->m_sprites[ index ],
                        enemy->getX(),
                        enemy->getY(),
                        0 );
    }
}


}
}
