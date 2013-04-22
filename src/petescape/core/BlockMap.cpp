
#include "petescape/core/BlockMap.h"

namespace petescape {
namespace core {

BlockMap::BlockMap(const GameMap &map) :
{
    height = map->getHeight();
    length = map->getLength();

    blockmap = (Block*) malloc( sizeof( Block ) * height * length);

    // copy values in
    for (int i = height; i >= 0; i--)
        for (int j = length; j >= 0; j--)
            addBlockAt(i, j, map->getValue(i, j));
}

BlockMap::~BlockMap() :
{
    free(blockmap);
}

Block getBlock(const uint32_t &x, const uint32_t &y)
{
    return blockmap[x * height + y];
}

void setBlock(const uint32_t &x, const uint32_t &y, Block &b)
{
    blockmap[x * height + y] = b;
}

void addBlockAt(const uint32_t &x, const uint32_t &y, uint16_t &val)
{
    setBlock(x, y, new Block(val));
}

}}
