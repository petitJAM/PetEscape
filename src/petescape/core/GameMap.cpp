
#include "petescape/core/GameMap.h"

namespace petescape {
namespace core {

GameMap::GameMap(const uint32_t &height, const uint32_t &length) :
    m_height( height ),
    m_length( length )
{
    generated = false,
    m_data = (uint16_t*)malloc( sizeof( uint16_t ) * m_height * m_length );
}

//frees the array inside the object - may not be necessary
GameMap::~GameMap()
{
    free(m_data);
}

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
// TODO add seed parameter
void GameMap::generate(){
    if (generated) {
        MESSAGE( "Warning: Map already generated, will not generate again.");
        return;
    }

    for(uint32_t i = 0; i < getLength(); i++)
    {
        for(uint32_t j = 0; j < getHeight(); j++)
        {
            if( j == getHeight() - 2 )
                setValue( i, j, 2 );

            else if( j == getHeight() - 1 )
                setValue( i, j, 7 );
            else if( i == 0 )
                setValue( i, j, 8 );
            else if( i == getLength() - 1 )
                setValue( i, j, 6 );

            else
                setValue( i, j, 0 );

            // I added this to generate walls and a floor for collision testing.
//            if( i == 0 || i == ( getLength() - 1) || j == 0 || j == ( getHeight() - 1 ) )
//                setValue( i, j, 1 );
//            else
//                setValue(i, j, 0);
        }
    }

//    for(uint32_t i = 0; i < getLength(); i++){
//        setValue(i, getHeight() - 1, 1);
//    }

    // seed rand
    // srand(123456);
    // srand(123456);

    // Seed with the current time.
    srand( time( nullptr ) );

    // populate with random platforms
    int n_plats = rand() % 150 + 100, plat_len, plat_x, plat_y;
    //n_plats = 0;

    if (m_length > 10)
    {
        for (int i = 0; i<n_plats; i++)
        {
            plat_len = rand() % 8;
            plat_x = (rand() % (m_length - 2/* - 10*/)) + 1/* + 5*/; // how far over
            plat_y = (rand() % (m_height - 6)) + 1; // how far up

            for (int j = 0; j<plat_len; j++)
            {
                setValue(plat_x + j, plat_y, 2);
            }
        }
    }
}

const uint16_t GameMap::getValue(const uint32_t &x, const uint32_t &y) const
{
    if(y >= m_height || x >= m_length){
        MESSAGE("INVALID ROW/COLUMN INPUT");
        return -1;
    }
    else{
        return m_data[ x * m_height + y ];
    }
}

void GameMap::setValue(const uint32_t &x, const uint32_t &y, const uint16_t &value)
{
    if(y >= m_height || x >= m_length){
        MESSAGE("INVALID ROW/COLUMN INPUT");
    }
    else{
        m_data[ x * m_height + y ] = value;
    }
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

        for( int i = 0; i < (getSize() - data.packet_number * MAP_PACKET_SIZE); i++){
            m_data[data.packet_number * MAP_PACKET_SIZE + i] = data.data_group[i];
        }
    }
}

void GameMap::populateChunk(map_data &data){
    //to be sure it's a full sized packet
    if( (data.packet_number + 1) * MAP_PACKET_SIZE < getSize() ){
        for(uint32_t i = 0; i < MAP_PACKET_SIZE; i++){
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

bool GameMap::isGenerated(){
    return generated;
}

void GameMap::display(){
    MESSAGE("GAMEMAP DISPLAY");
    for(uint32_t i = 0; i < getHeight(); i++){
        for(uint32_t j = 0; j < getLength(); j++){
            printf("%d", (int)getValue(j, i));
        }
        printf("\n");
    }
}

}}
