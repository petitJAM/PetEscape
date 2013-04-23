
#include "petescape/core/Block.h"

namespace petescape {
namespace core {

Block::Block(const uint16_t &blockVal) :
    blockVal( blockVal )
{
    // TODO
    type =      blockVal        & 0x0F;
    subtype =  (blockVal >> 4)  & 0x0F;
    rotation = (blockVal >> 14) & 0x03;
}

Block::~Block()
{
    // empty for now
}

const uint16_t Block::getBlockVal(){
    return blockVal;
}

const uint8_t Block::getBlockType(){
    return type;
}

const uint8_t Block::getBlockSubType(){
    return subtype;
}

const uint8_t Block::getBlockRotation(){
    return rotation;
}



}
}
