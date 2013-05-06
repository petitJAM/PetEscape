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

/**
 *  Handshake messages structures.
 */
typedef struct CLIENT_HELLO
{
    int8_t      client_ip[16];
} client_hello;

typedef struct CLIENT_USER_INPUT
{
    ALLEGRO_EVENT event;
    time_t event_time;
} user_input;

typedef struct PADDING_PACKET
{
    char        padding;
} client_close, server_ready;

typedef struct SERVER_INFO
{
    uint32_t    client_id;
} server_info;

/**
 *  Server messages structures
 */
typedef struct MAP_HEADER
{
    uint32_t    stage_length;
    uint32_t    stage_height;
} map_header;

typedef struct MAP_DATA
{
    uint8_t     packet_number;
    uint16_t     data_group[MAP_PACKET_SIZE];
} map_data;

typedef struct UPDATE_OBJ
{
    uint32_t    id;
    uint32_t    x;
    uint32_t    y;
    uint16_t    type;
    uint8_t     action;
    uint8_t     facing;
    uint8_t     walk_phase;
} update_obj, introduce_obj;

typedef struct DESTROY_OBJ
{
    uint32_t    id;
    uint16_t    type;
} destroy_obj;

/**
 *  Client messages structures
 */
typedef struct PLAYER_MOVE
{
    uint32_t    id;
    uint8_t     direction;
    uint8_t     start_stop;
} player_move;

typedef struct PLAYER_JUMP
{
    uint32_t    id;
    uint8_t     start_stop;
} player_jump;

typedef struct PLAYER_ATTACK
{
    uint32_t    id;
    uint8_t     type;
} player_attack;

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
    C_READY,
    C_REQUEST_MAP,
    C_BUILD_OBJECTS,
    C_USER_INPUT,
    S_INFO,
    S_MAP_HEADER,
    S_MAP_DATA,
    S_READY,
    O_UPDATE,
    O_INTRODUCE,
    O_DESTORY,
    P_MOVE,
    P_JUMP,
    P_ATTACK
} packet_id;

typedef union PACKET_LIST
{
    client_hello        c_hello;
    user_input          c_user_input;
    client_close        c_close;
    server_info         s_info;
    map_header          s_map_header;
    map_data            s_map_data;
    update_obj          o_update;
    introduce_obj       o_introduce;
    destroy_obj         o_destroy;
    player_move         p_move;
    player_jump         p_jump;
    player_attack       p_attack;
    server_ready        s_ready;
} packet_list;

typedef struct NETWORK_PACKET
{
    packet_header   head;
    packet_list     data;
    void*           data2;
} network_packet;


} // End Common Namespace
} // End Networking Namespace
} // End PetEscape Namespace

#endif // NETWORK_PACKET_H
