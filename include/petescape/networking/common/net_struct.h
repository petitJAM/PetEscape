#ifndef NETWORK_PACKET_H
#define NETWORK_PACKET_H 1

#define MAP_PACKET_SIZE 100

#include <stdint.h>
#include <petescape/core/core_defs.h>
#include <allegro5/allegro.h>

/**
 * Get into the right namespace.
 */
namespace petescape {
namespace networking {
namespace common {

/************************
 *  Command structures  *
 ************************/

struct PADDING_PACKET
{
    uint8_t     padding;
};

struct ID_PACKET
{
    uint32_t    client_id;
};

struct MAP_HEADER
{
    uint32_t    stage_length;
    uint32_t    stage_height;
};

struct MAP_DATA
{
    uint8_t     packet_number;
    uint16_t    data_group[MAP_PACKET_SIZE];
};

struct OBJECT_INFO
{
    uint32_t    id;
    uint8_t     p_id;
    uint32_t    x;
    uint32_t    y;
    uint16_t    type;
    uint16_t    second_type;
    uint8_t     action;
    uint8_t     facing;
    uint8_t     walk_phase;
};

struct DESTROY_OBJ
{
    uint32_t    id;
    uint16_t    type;
};

/*
 *  Typedefs for structs.
 */
typedef struct PADDING_PACKET   padding_packet;
typedef struct PADDING_PACKET   client_request_map;
typedef struct PADDING_PACKET   client_request_objs;
typedef struct PADDING_PACKET   server_sent_map;
typedef struct PADDING_PACKET   server_sent_objs;
typedef struct PADDING_PACKET   client_hello;

typedef struct ID_PACKET        server_info;
typedef struct ID_PACKET        client_close;
typedef struct MAP_DATA         map_data;
typedef struct MAP_HEADER       map_header;
typedef struct OBJECT_INFO      update_obj;
typedef struct DESTROY_OBJ      destroy_obj;

/***************************
 *  High-level structures  *
 ***************************/

/**
 * Struct that holds information about the data packet.
 * Should be exactly 32 bits.
 */
typedef struct PACKET_HEADER
{
    /**
     * @brief opcode Information about what to do with the data.
     */
    uint16_t opcode;

    /**
     * @brief response_required Self explanitory
     */
    uint8_t response_required;

    /**
     * @brief sender_id The sender. 0 is the server.
     */
    uint8_t sender_id;

} packet_header;

typedef enum PACKET_ID
{
    DATA_NULL = 0x0000,
    C_HELLO = 0x0001,
    C_CLOSE,
    C_REQUEST_MAP,
    C_REQUEST_OBJS,
    S_INFO,
    S_MAP_HEADER,
    S_MAP_DATA,
    S_SENT_MAP,
    S_SENT_OBJS,
    O_UPDATE,
    O_DESTORY,
    E_UPDATE
} packet_id;

typedef union PACKET_LIST
{
    client_hello        c_hello;
    client_close        c_close;
    client_request_map  c_req_map;
    client_request_objs c_req_objs;
    server_info         s_info;
    server_sent_map     s_sent_map;
    server_sent_objs    s_sent_objs;
    map_header          s_map_header;
    map_data            s_map_data;
    update_obj          o_update;
    update_obj          e_update;
    destroy_obj         o_destroy;
} packet_list;

typedef struct NETWORK_PACKET
{
    packet_header   head;
    packet_list     data;
} network_packet;


} // End Common Namespace
} // End Networking Namespace
} // End PetEscape Namespace

#endif // NETWORK_PACKET_H
