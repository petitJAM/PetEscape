#include "petescape/networking/client/ClientConnection.h"

using petescape::networking::client::ClientConnection;
using namespace petescape::networking::common;

ClientConnection::ClientConnection( boost::asio::io_service &io_s, uint32_t id )
    : common::TCP_Connection( io_s, id )
{
    this->m_event_dispatcher = nullptr;
}

void ClientConnection::begin()
{
/*    // Let the server know that we want to talk with it.
    packet_list packet;
    packet_id   id;

    // Let's get this shit going. Write some stuff to the server.

    // TODO: Something is wrong with this line. probably something stupid.
    //       I'll fix it later. Don't need it now.
//    strncpy( "unused", (char*)packet.c_hello.client_ip, sizeof( packet.c_hello.client_ip ) );
    sync_write( packet, C_HELLO );

    sync_read( packet, id );

    if( id != H_REQUEST ) {
        std::cerr << "Server didn't respond with handshake request.\n";
        exit( -1 );
    }

    if( packet.h_accept.code != SERVER_HANDSHAKE_VALUE ) {
        std::cerr << "Server responded with unexpected handshake code.\n";
        exit( -2 );
    }

    memset( &packet, 0, sizeof( packet ) );
    packet.h_accept.code = CLIENT_HANDSHAKE_VALUE;
    sync_write( packet, H_ACCEPT );

    sync_read( packet, id );

    if( id != S_INFO ) {
        std::cerr << "Server did not accept the handshake.\n";
        exit( -3 );
    }

    setID( packet.s_info.client_id );

    memset( &packet, 0, sizeof( packet ) );
    packet.c_accept.client_id = this->m_id;

    std::cout << "good?\n";

    sync_write( packet, C_ACCEPT );
*/
//#ifdef DEBUG
    std::cout << "Completed handshake with server.\n";
    std::cout << "Begining asyc operations...\n";
//#endif

    async_read();
}

void ClientConnection::read_callback( const boost::system::error_code &error,
                                      size_t /*read_count*/ )
{

    if( error )
    {
        std::cerr << "Error reading: " << error.message() << std::endl;
        return;
    }

    std::cerr << "Client received packet...\n";

    // Handle incoming packet asynchronously
    if( this->m_event_dispatcher != nullptr )
    {
        std::cerr << "Client dispatching packet\n";
        ALLEGRO_EVENT event;
        event.type = 512;
        event.user.data1 = (intptr_t)&m_input;

        al_emit_user_event( this->m_event_dispatcher, &event, nullptr );
    }
    else
    {
        std::cerr << "Client failed to dispatch packet\n";
    }

    async_read();
}

void ClientConnection::write_callback(const boost::system::error_code &error,
                                      size_t /*write_count*/ )
{
    // After a write we don't need to do anything except error checking
    if( error )
    {
        std::cerr << "Error writing to server. " << error.message() << std::endl;
    }
    else
    {
        std::cerr << "Write successful." << std::endl;
    }
}


void ClientConnection::sync_write(const packet_list &packet, packet_id id, uint8_t rr)
{

    this->m_output.head.opcode = id;
    this->m_output.head.response_required = rr;
    this->m_output.head.sender_id = this->m_id;

    this->m_output.data = packet;

    boost::asio::write( this->m_socket,
                        boost::asio::buffer( &m_output, sizeof( m_output ) ) );
}


void ClientConnection::sync_read(packet_list &packet, packet_id &id)
{
    boost::asio::read( this->m_socket,
                       boost::asio::buffer( &m_input, sizeof( m_input ) ) );

    packet = m_input.data;
    id = (packet_id)m_input.head.opcode;
}

void ClientConnection::async_read()
{
    boost::asio::async_read( this->m_socket,
                             boost::asio::buffer( &m_input, sizeof( m_input ) ),
                             boost::bind( &ClientConnection::read_callback,
                                          this,
                                          boost::asio::placeholders::error,
                                          boost::asio::placeholders::bytes_transferred ) );

}

void ClientConnection::async_write(const packet_list &packet, packet_id id, uint8_t rr)
{
    this->m_output.data = packet;
    this->m_output.head.opcode = id;
    this->m_output.head.sender_id = this->m_id;
    this->m_output.head.response_required = rr;

    std::cout << "WRITING!" << std::endl;

    boost::asio::async_write( this->m_socket,
                               boost::asio::buffer( &m_output, sizeof( m_output ) ),
                               boost::bind( &ClientConnection::write_callback,
                                            this,
                                            boost::asio::placeholders::error,
                                            boost::asio::placeholders::bytes_transferred ) );
}
