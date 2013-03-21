#ifndef NETWORK_PACKET_H
#define NETWORK_PACKET_H 1

#include <stdint.h>

/**
 * Get into the right namespace.
 */
namespace petescape
{
namespace networking
{
namespace common
{

/**
 * Struct that holds information about the data packet.
 * Should be exactly 64 bits.
 */
struct packet_header
{
    /**
     * @brief opcode Information about what to do with the data.
     */
    uint16_t opcode;

    /**
     * @brief data_format Tells us the union to use.
     */
    uint8_t data_format;

    /**
     * @brief response_required Self explanitory
     */
    uint8_t response_required;

    /**
     * @brief sender_id The sender. 0 is the server.
     */
    uint8_t sender_id;

    /**
     * @brief padding Availble bytes for later use.
     */
    uint8_t padding[4];
};

struct network_packet
{
    struct packet_header head;

    uint32_t msg;

//    union
//    {
//        struct
//        {
//            unsigned int v1;
//            unsigned int v2;
//        } t1;

//        struct
//        {
//            signed int v1;
//            signed int v2;
//        } t2;

//    } data;
};

struct string_net_packet
{
    struct packet_header head;

    int8_t data[32];
};

typedef union NET_PACKET_GROUP
{
    struct packet_header header;
    struct network_packet standard;
    struct string_net_packet str_packet;
} packet_list;

} // End Common Namespace
} // End Networking Namespace
} // End PetEscape Namespace

#endif // NETWORK_PACKET_H
