#ifndef OBJECTRENDERER_H
#define OBJECTRENDERER_H

#include <allegro5/allegro.h>

namespace petescape {
namespace core {

class GameObject;

class ObjectRenderer
{
public:
    ObjectRenderer(){}

    virtual void render( GameObject * ) = 0;
};

class PoorRenderer : public ObjectRenderer
{
public:
    PoorRenderer(){}

    void render( GameObject * );
};

class PlayerRenderer : public ObjectRenderer
{
public:
    PlayerRenderer( ALLEGRO_BITMAP *images[ 8 ] );

    void render( GameObject * );

private:
    ALLEGRO_BITMAP *m_sprites[ 8 ];

};

}}

#endif // OBJECTRENDERER_H
