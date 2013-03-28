#ifndef OBJECTRENDERER_H
#define OBJECTRENDERER_H

namespace petescape {
namespace core {

class GameObject;

class ObjectRenderer
{
public:
    ObjectRenderer(){}

    virtual void render( const GameObject * ) = 0;
};

}
}

#endif // OBJECTRENDERER_H
