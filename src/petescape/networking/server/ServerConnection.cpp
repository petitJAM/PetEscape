#include "petescape/networking/server/ServerConnection.h"

#include <ctime>

using petescape::networking::server::ServerConnection;

void ServerConnection::begin()
{
/*
    packet_list packet;
    packet_id   id;
    uint8_t     c_id;

    // Shit just got real. We have a new connection.
    // Wait for a hello, then offer a handshake.

    sync_read( packet, id );

    if( id != C_HELLO )
    {
        std::cerr << "Unexpected packet from client.\n";
        exit( -1 );
    }

    // Respond to the accepted handshake with
    // server info
    memset( &packet, 0, sizeof( packet ) );
    packet.h_request.code = SERVER_HANDSHAKE_VALUE;

    sync_write( packet, H_REQUEST );
    memset( &packet, 0, sizeof( packet ) );

    // Make sure the client knows its ID.
    sync_read( packet, id );

    if( id != H_ACCEPT )
    {
        std::cerr << "Unexpected response from client." << std::endl;
        exit( -2 );
    }

    if( packet.h_accept.code != CLIENT_HANDSHAKE_VALUE )
    {
        std::cerr << "Client failed to accept the handshake." << std::endl;
        exit( -3 );
    }

    memset( &packet, 0, sizeof( packet ) );
    packet.s_info.client_id = c_id = GenClientID();
    packet.s_info.server_base_time = this->m_server_start_time;

    sync_write( packet, S_INFO );

    sync_read( packet, id );

    if( id != C_ACCEPT )
    {
        std::cerr << "Unexpected response from client." << std::endl;
        exit( -4 );
    }

    if( packet.c_accept.client_id != c_id )
    {
        std::cerr << "Client failed to return its ID." << std::endl;
        exit( -5 );
    }

    std::cerr << "Client is legit.\n";
*/

    // Begin async operations, Client is legit.
    std::cerr << "Reading on server...\n";

    async_read();
}

void ServerConnection::read_callback( const boost::system::error_code &error,
                                      size_t /*read_count*/ )
{
    if( error )
    {
        std::cerr << "Error reading: " << error.message() << std::endl;
        return;
    }

    std::cerr << "Server reading data...\n";

    if( this->m_event_dispatcher != nullptr )
    {
        ALLEGRO_EVENT event;

        if( m_input.head.opcode == C_HELLO )
        {
            event.type = 513;
        }
        else
        {
            event.type = 512;
            event.user.data1 = (intptr_t)&m_input;
        }

        al_emit_user_event( this->m_event_dispatcher, &event, nullptr );
    }
    else
    {
        std::cerr << "Why is this null....\n";
    }

    async_read();
}

void ServerConnection::write_callback(const boost::system::error_code &error,
                                      size_t /*write_count*/ )
{
    // After a write, we don't need to do anything except error checking.
    if( error.value() )
    {
        std::cerr << "Error writing to client. " << error.message() << std::endl;
    }
}

void ServerConnection::sync_write( const packet_list &packet, packet_id id, uint8_t rr )
{
    this->m_output.head.opcode = id;
    this->m_output.head.response_required = rr;
    this->m_output.head.sender_id = id;

    this->m_output.data = packet;

    boost::asio::write( this->m_socket,
                        boost::asio::buffer( &m_output, sizeof( m_output ) ) );
}

void ServerConnection::sync_read()
{
    boost::system::error_code error;

//    boost::asio::read( this->m_socket,
//                            boost::asio::buffer( &m_input, sizeof( m_input ) ),
//                            boost::bind( &ServerConnection::read_callback,
//                                         shared_from_this(),
//                                         boost::asio::placeholders::error,
//                                         boost::asio::placeholders::bytes_transferred ) );

    boost::asio::read( this->m_socket,
                       boost::asio::buffer( &m_input, sizeof( m_input ) ),
                       error );

    read_callback(error, 0);
}

void ServerConnection::sync_read( packet_list &packet, packet_id &id )
{
    boost::asio::read( this->m_socket,
                       boost::asio::buffer( &m_input, sizeof( m_input ) ) );

    packet = m_input.data;
    id = (packet_id)m_input.head.opcode;
}

void ServerConnection::async_read()
{
    if( this->m_socket.is_open() == false )
    {
        std::cerr << "socket closed... (read)\n";
    }

    boost::asio::async_read( this->m_socket,
                             boost::asio::buffer( &m_input, sizeof( m_input ) ),
                             boost::bind( &ServerConnection::read_callback,
                                          shared_from_this(),
                                          boost::asio::placeholders::error,
                                          boost::asio::placeholders::bytes_transferred ) );
}

void ServerConnection::async_write(const packet_list &packet, packet_id id, uint8_t rr)
{
    this->m_output.data = packet;
    this->m_output.head.opcode = id;
    this->m_output.head.sender_id = 0;
    this->m_output.head.response_required = rr;

    if( this->m_socket.is_open() )
    {
        std::cerr << "socket closed... (write)\n";
    }

    boost::asio::async_write( this->m_socket,
                              boost::asio::buffer( &m_output, sizeof( m_output ) ),
                              boost::bind( &ServerConnection::write_callback,
                                           shared_from_this(),
                                           boost::asio::placeholders::error,
                                           boost::asio::placeholders::bytes_transferred ) );
}

void ServerConnection::setEventSource(ALLEGRO_EVENT_SOURCE *src)
{
    this->m_event_dispatcher = src;
}
