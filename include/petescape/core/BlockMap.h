#ifndef BLOCKMAP_H
#define BLOCKMAP_H

#include "petescape/core/Block.h"
#include "petescape/core/GameMap.h"

namespace petescape {
namespace core {

class BlockMap
{
public:
    BlockMap( GameMap &map );

    ~BlockMap();

    inline const uint32_t getHeight();
    inline const uint32_t getLength();
    Block getBlock( const uint32_t &x, const uint32_t &y );
    inline void setBlock( const uint32_t &x, const uint32_t &y, Block &b);

    void addBlockAt( const uint32_t &x, const uint32_t &y, const uint16_t &val);

    void display();

private:
    uint32_t    height;
    uint32_t    length;
    Block     **blockmap;
};

}}

#endif // BLOCKMAP_H
