#ifndef BLOCKMAP_H
#define BLOCKMAP_H

namespace petescape {
namespace core {

class BlockMap
{
public:
    BlockMap( const GameMap &map );

    ~BlockMap();

    inline Block getBlock( uint32_t &x, uint32_t &y );

private:
    uint32_t    height;
    uint32_t    length;
    Block      *blockmap;
};

}}

#endif // BLOCKMAP_H
