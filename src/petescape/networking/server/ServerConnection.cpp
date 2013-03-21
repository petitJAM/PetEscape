#include "petescape/networking/server/ServerConnection.h"

using petescape::networking::server::ServerConnection;

void ServerConnection::begin()
{
    // Doesn't have to do anything. Waits for a client to connect.
    boost::asio::async_read( this->m_socket,
                             boost::asio::buffer( &m_input, sizeof( m_input ) ),
                             boost::bind( &ServerConnection::read_callback,
                                          shared_from_this(),
                                          boost::asio::placeholders::error,
                                          boost::asio::placeholders::bytes_transferred ) );
}

void ServerConnection::read_callback( const boost::system::error_code &/*error*/,
                                      size_t /*read_count*/ )
{
    // Do stuff with client input.

    boost::asio::async_write( this->m_socket,
                              boost::asio::buffer( &m_output, sizeof( m_output ) ),
                              boost::bind( &ServerConnection::write_callback,
                                           shared_from_this(),
                                           boost::asio::placeholders::error,
                                           boost::asio::placeholders::bytes_transferred ) );
}

void ServerConnection::write_callback(const boost::system::error_code &/*error*/,
                                      size_t /*write_count*/ )
{
    // Wait for a read now.
    // Write can be called from other places,
    // so we don't need to call it explicitly here.

    boost::asio::async_read( this->m_socket,
                             boost::asio::buffer( &m_input, sizeof( m_input ) ),
                             boost::bind( &ServerConnection::read_callback,
                                          shared_from_this(),
                                          boost::asio::placeholders::error,
                                          boost::asio::placeholders::bytes_transferred ) );
}
