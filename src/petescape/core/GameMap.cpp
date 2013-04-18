
#include "petescape/core/GameMap.h"

namespace petescape {
namespace core {

GameMap::GameMap(const uint32_t &height, const uint32_t &length) :
    m_height( height ),
    m_length( length )
{
    m_data = (uint8_t*)malloc( sizeof( uint8_t ) * m_height * m_length );
}

//frees the array inside the object - may not be necessary
GameMap::~GameMap()
{
    free(m_data);
}

void GameMap::generate(){
    for(uint32_t i = 0; i < getSize(); i++){
        if(i % m_length == m_length - 1)
            m_data[i] = 1;
        else
            m_data[i] = 0;
    }
}

const uint8_t GameMap::getValue(const uint32_t &row, const uint32_t &column) const
{
    if(row >= m_height || column >= m_length){
        //MESSAGE("INVALID ROW/COLUMN INPUT");
        return -1;
    }
    else{
        return m_data[ column * m_length + row ];
    }
}

void GameMap::setValue(const uint32_t &row, const uint32_t &column, const uint8_t &value)
{
    if(row >= m_height || column >= m_length){
        //MESSAGE("INVALID ROW/COLUMN INPUT");
    }
    else{
        m_data[ column * m_length + row ] = value;
    }
}

void GameMap::addChunk(const map_data &data)
{
    //to be sure it's a full sized packet
    if((data.packet_number + 1) * MAP_PACKET_SIZE < getSize()){
        for( uint32_t i = 0; i < MAP_PACKET_SIZE; ++i )
        {
            m_data[ MAP_PACKET_SIZE * data.packet_number + i] = data.data_group[ i ];
        }
    }
    else{

        for(uint32_t i = 0; i < (getSize() - data.packet_number * MAP_PACKET_SIZE); i++){
            m_data[data.packet_number * MAP_PACKET_SIZE + i] = data.data_group[i];
        }
    }
}

void GameMap::populateChunk(map_data &data){
    //to be sure it's a full sized packet
    if((data.packet_number + 1) * MAP_PACKET_SIZE < getSize()){
        MESSAGE("ENTERING ME");
        for(uint32_t i = 0; i < MAP_PACKET_SIZE; i++){
            MESSAGE((int)(m_data[data.packet_number * MAP_PACKET_SIZE + i]));
            data.data_group[i] = m_data[data.packet_number * MAP_PACKET_SIZE + i];
        }
    }
    else {
        for(uint32_t i = 0; i < (getSize() - data.packet_number * MAP_PACKET_SIZE); i++){
            data.data_group[i] = m_data[data.packet_number * MAP_PACKET_SIZE + i];
        }
    }
}

const uint8_t GameMap::getHeight(){
    return m_height;
}

const uint8_t GameMap::getLength(){
    return m_length;
}

const size_t GameMap::getSize(){
    return m_height*m_length/**sizeof(uint8_t)*/;
}

}}
