
#include "petescape/core/BlockMap.h"
#include "petescape/core/Block.h"

namespace petescape {
namespace core {

BlockMap::BlockMap(GameMap &map) :
    height( map.getHeight() ),
    length( map.getLength() )
{

    blockmap = (Block**) malloc( sizeof( Block* ) * height * length);

    // copy values in
    for (uint32_t i = 0; i < length; i++)
        for (uint32_t j = 0; j < height; j++)
            addBlockAt(i, j, map.getValue(i, j));
}

BlockMap::~BlockMap()
{
    // fix
    free(blockmap);
}

const uint32_t BlockMap::getHeight()
{
    return height;
}

const uint32_t BlockMap::getLength()
{
    return length;
}

Block BlockMap::getBlock(const uint32_t &x, const uint32_t &y)
{
    return *blockmap[x * height + y];
}

//void BlockMap::setBlock(const uint32_t &x, const uint32_t &y, Block &b)
//{
//    blockmap[x * height + y] = b;
//}

void BlockMap::addBlockAt(const uint32_t &x, const uint32_t &y, const uint16_t &val)
{
    Block* b = new Block(val);
    blockmap[x * height + y] = b;
}

void BlockMap::display()
{
    MESSAGE("BLOCKMAP DISPLAY");
    for(uint32_t i = 0; i < getHeight(); i++){
        for(uint32_t j = 0; j < getLength(); j++){
            printf("%d", (int)getBlock(j, i).getBlockType());
        }
        printf("\n");
    }
}

}}
