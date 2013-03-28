#ifndef NETWORK_PACKET_H
#define NETWORK_PACKET_H 1

#include <stdint.h>

/**
 * Get into the right namespace.
 */
namespace petescape {
namespace networking {
namespace common {

#define SERVER_HANDSHAKE_VALUE 0x1234CDEF
#define CLIENT_HANDSHAKE_VALUE 0xFEDC4321

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

typedef struct HANDSHAKE
{
    uint32_t    code;
} handshake_request, handshake_accept;

typedef struct SERVER_INFO
{
    uint32_t    client_id;
    uint64_t    server_base_time;
} server_info;

typedef struct CLIENT_ACCEPT
{
    uint32_t    client_id;
} client_accept;

/**
 *  Server messages structures
 */
typedef struct UPDATE_OBJ
{
    uint32_t    id;
    uint32_t    x;
    uint32_t    y;
    uint8_t     action;
    uint8_t     facing;
} update_obj, introduce_obj;

typedef struct DESTROY_OBJ
{
    uint32_t    id;
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
    H_REQUEST,
    H_ACCEPT,
    S_INFO,
    C_ACCEPT,
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
    handshake_request   h_request;
    handshake_accept    h_accept;
    server_info         s_info;
    client_accept       c_accept;
    update_obj          o_update;
    introduce_obj       o_introduce;
    destroy_obj         o_destroy;
    player_move         p_move;
    player_jump         p_jump;
    player_attack       p_attack;
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