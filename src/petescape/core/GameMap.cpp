
#include "petescape/core/GameMap.h"

namespace petescape {
namespace core {

GameMap::GameMap(const uint32_t &row_count, const uint32_t &col_count) :
    m_row_count( row_count ),
    m_col_count( col_count )
{
    m_data = (uint8_t*)malloc( sizeof( uint8_t ) * m_row_count * m_col_count );
}

//frees the array inside the object - may not be necessary
GameMap::~GameMap()
{
    free(m_data);
}

void GameMap::generate(){
    for(int i = 0; i < m_row_count - 1; i++){
        for(int j = 0; j < m_col_count; j++){
            setValue(i, j, 0);
        }
    }

    for(int j = 0; j < m_col_count; j++){
        setValue(m_row_count - 1, j, 1);
    }
}

const uint8_t GameMap::getValue(const uint32_t &row, const uint32_t &column) const
{
    return m_data[ row + column * m_col_count ];
}

void GameMap::setValue(const uint32_t &row, const uint32_t &column, const uint8_t &value)
{
    m_data[ row + column * m_col_count ] = value;
}

void GameMap::addChunk(const map_data &data)
{
    //to be sure it's a full sized packet
    if((data.packet_number + 1) * MAP_PACKET_SIZE < getSize()){
        for( int i = 0; i < MAP_PACKET_SIZE; ++i )
        {
            m_data[ MAP_PACKET_SIZE * data.packet_number + i] = data.data_group[ i ];
        }
    }
    else{

        for(int i = 0; i < (getSize() - data.packet_number * MAP_PACKET_SIZE); i++){
            m_data[data.packet_number * MAP_PACKET_SIZE + i] = data.data_group[i];
        }
    }
}

void GameMap::populateChunk(map_data &data){
    //to be sure it's a full sized packet
    if((data.packet_number + 1) * MAP_PACKET_SIZE < getSize()){
        for(int i = 0; i < MAP_PACKET_SIZE; i++){
            data.data_group[i] = m_data[data.packet_number * MAP_PACKET_SIZE + i];
        }
    }
    else {
        for(int i = 0; i < (getSize() - data.packet_number * MAP_PACKET_SIZE); i++){
            data.data_group[i] = m_data[data.packet_number * MAP_PACKET_SIZE + i];
        }
    }
}

const uint8_t GameMap::getHeight(){
    return m_row_count;
}

const uint8_t GameMap::getLength(){
    return m_col_count;
}

const size_t GameMap::getSize(){
    return m_row_count*m_col_count*sizeof(uint8_t);
}

}}
