#ifndef OBJECTRENDERER_H
#define OBJECTRENDERER_H

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

}
}

#endif // OBJECTRENDERER_H
