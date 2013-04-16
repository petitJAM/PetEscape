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
    GameMap( const uint32_t &row_count, const uint32_t &col_count );

    inline const uint8_t getValue( const uint32_t &row, const uint32_t &column ) const;

    inline void setValue( const uint32_t &row, const uint32_t &column, const uint8_t &value );

    void addChunk( const map_data &data );

private:
    uint32_t    m_row_count;
    uint32_t    m_col_count;
    uint8_t     *m_data;
};

}}

#endif // GAMEMAP_H
