#include "petescape/networking/client/ClientConnection.h"

using petescape::networking::client::ClientConnection;

void ClientConnection::begin()
{
    // Let the server know that we want to talk with it.
    m_input.header.opcode = 0xffff;
    m_input.header.data_format = 0x00;
    m_input.header.response_required = 0x00;
    m_input.header.sender_id = 0xff;

    boost::asio::async_write( this->m_socket,
                              boost::asio::buffer( &m_input, sizeof( m_input ) ),
                              boost::bind( &ClientConnection::write_callback,
                                           shared_from_this(),
                                           boost::asio::placeholders::error,
                                           boost::asio::placeholders::bytes_transferred ) );
}

void ClientConnection::read_callback( const boost::system::error_code &/*error*/,
                                      size_t /*read_count*/ )
{
    // Do stuff with server input.

    // If a response is needed
    if( m_input.header.response_required )
    {
        boost::asio::async_write( this->m_socket,
                              boost::asio::buffer( &m_output, sizeof( m_output ) ),
                              boost::bind( &ClientConnection::write_callback,
                                           shared_from_this(),
                                           boost::asio::placeholders::error,
                                           boost::asio::placeholders::bytes_transferred ) );
    }
}

void ClientConnection::write_callback(const boost::system::error_code &/*error*/,
                                      size_t /*write_count*/ )
{
    // Wait for a read now.
    // Write can be called from other places,
    // so we don't need to call it explicitly here.

    boost::asio::async_read( this->m_socket,
                             boost::asio::buffer( &m_input, sizeof( m_input ) ),
                             boost::bind( &ClientConnection::read_callback,
                                          shared_from_this(),
                                          boost::asio::placeholders::error,
                                          boost::asio::placeholders::bytes_transferred ) );
}
