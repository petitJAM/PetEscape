#ifndef BLOCK_H
#define BLOCK_H

#include <stdint.h>

namespace petescape {
namespace core {

class Block
{

public:
    Block( const uint16_t &blockVal );

    ~Block();

    // will probably not need this
    inline const uint16_t getBlockVal();

    inline const uint8_t getBlockType();
    inline const uint8_t getBlockSubType();
    inline const uint8_t getBlockRotation();

private:
    // the original input
    uint16_t    blockVal;

    // what kind of block is it? grass, air, etc...
    uint8_t     type;

    // grass on top, grass on two sides/three sides
    uint8_t     subtype;

    // what rotation should this be?
    uint8_t     rotation;
};
}}

#endif // BLOCK_H
