
#include "petescape/core/GameMap.h"

namespace petescape {
namespace core {

GameMap::GameMap(const uint32_t &row_count, const uint32_t &col_count) :
    m_row_count( row_count ),
    m_col_count( col_count )
{
    generated = false,
    m_data_length = m_row_count * m_col_count,
    m_data = (uint8_t*)malloc( sizeof( uint8_t ) * m_row_count * m_col_count );
}

// gens map like this (with different size):
// 00000
// 00000
// 11111


/*
 *  Generate the data for this map using (not yet) the given seed
 *
 *  Maps look like this:
 *  00000
 *  00000
 *  11111
 *
 *  With indexing like so:
 *  0  4  8
 *  1  5  9
 *  2  6  10
 *  3  7  11
 *
 *  This is done since our maps will most likely have varying lengths,
 *  but mostly fixed heights.
 *
 *  Ignored:
 *      @param - seed
 */
void GameMap::generate(uint32_t seed)
{
    if (generated) {
        MESSAGE( "Warning: Map already generated, will not generate again.");
        return;
    }

    // TODO update this...
    for (uint32_t i = 0; i < m_data_length; i++)
    {
        if (i % m_col_count == m_col_count - 1)
            m_data[i] = 1;
        else
            m_data[i] = 0;
    }

    // seed rand
    // srand(123456);
    srand(123456);

    // populate with random platforms
    int n_plats = rand() % 50, plat_len, plat_x, plat_y;

    if (m_col_count > 10)
    {
        for (int i = 0; i<n_plats; i++)
        {
            plat_len = rand() % 5;
            plat_x = (rand() % (m_col_count - 10)) + 5; // how far over
            plat_y = (rand() % (m_row_count - 10)) + 2; // how far up

            for (int j = 0; j<plat_len; j++)
            {
                // TODO update this too...
                m_data[plat_y + ((plat_x + j - 1) * m_row_count)] = 1;
            }
        }
    }
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
