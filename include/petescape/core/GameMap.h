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

    const uint8_t getValue( const uint32_t &row, const uint32_t &column ) const;

    inline void setValue( const uint32_t &row, const uint32_t &column, const uint8_t &value );

    void addChunk( const map_data &data );

    void populateChunk(map_data &data);

    inline const uint8_t getHeight();
    inline const uint8_t getLength();
    inline const size_t getSize();

private:
    uint32_t    m_height;
    uint32_t    m_length;
    uint8_t     *m_data;
};

}}

#endif // GAMEMAP_H
