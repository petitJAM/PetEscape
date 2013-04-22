#ifndef GAMEMAP_H
#define GAMEMAP_H

#include <stdint.h>
#include "petescape/networking/common/net_struct.h"

namespace petescape {
namespace core {

using namespace petescape::networking::common;

class GameMap
{

public:
    GameMap( const uint32_t &height, const uint32_t &length );

    ~GameMap();

    void generate();
    //void generate(uint32_t seed);

    const uint8_t getValue( const uint32_t &x, const uint32_t &y ) const;

    inline void setValue( const uint32_t &x, const uint32_t &y, const uint16_t &value );

    void addChunk( const map_data &data );

    void populateChunk(map_data &data);

    inline const uint8_t getHeight();
    inline const uint8_t getLength();
    inline const size_t getSize();

    inline const uint8_t getId();

    void display();

private:
    uint32_t    m_height;
    uint32_t    m_length;
    uint16_t     *m_data;
    uint8_t     id;
    bool        generated;
};

}}

#endif // GAMEMAP_H
