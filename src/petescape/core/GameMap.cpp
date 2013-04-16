
#include "petescape/core/GameMap.h"

namespace petescape {
namespace core {

GameMap::GameMap(const uint32_t &row_count, const uint32_t &col_count) :
    m_row_count( row_count ),
    m_col_count( col_count )
{
    m_data = (uint8_t*)malloc( sizeof( uint8_t ) * m_row_count * m_col_count );
}

const uint8_t GameMap::getValue(const uint32_t &row, const uint32_t &column) const
{
    return m_data[ row * m_row_count + column ];
}

void GameMap::setValue(const uint32_t &row, const uint32_t &column, const uint8_t &value)
{
    m_data[ row * m_row_count + column ] = value;
}

void GameMap::addChunk(const map_data &data)
{
    for( int i = 0; i < MAP_PACKET_SIZE; ++i )
    {
        m_data[ MAP_PACKET_SIZE * data.packet_number + data.data_group[ i ] ];
    }
}

}}
