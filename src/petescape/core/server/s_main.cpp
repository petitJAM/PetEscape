
#include "petescape/core/server/server.h"
#include "petescape/networking/server/ServerConnection.h"

#include <boost/bind.hpp>
#include <boost/asio.hpp>

namespace petescape {
namespace core {
namespace server {

using petescape::networking::server::ServerConnection;

PetEscapeServer::PetEscapeServer(boost::asio::io_service &io) :
    m_acceptor( io, tcp::endpoint( tcp::v4(), 2001 ) )
{
    // TODO: Set this for real.
    this->m_start_time = 0;
    this->begin_accept( io );
}

void PetEscapeServer::begin_accept( boost::asio::io_service &io )
{
    ServerConnection::server_conn_ptr server_ptr =
            ServerConnection::CreateConnection( io, 0 );

    server_ptr->setStartTime( this->m_start_time );

    this->m_acceptor.async_accept( server_ptr->getSocket(),
                                   boost::bind( &PetEscapeServer::handle_accept,
                                                this,
                                                server_ptr,
                                                boost::asio::placeholders::error ) );
}

void PetEscapeServer::handle_accept( ServerConnection::server_conn_ptr server_conn,
                                     const boost::system::error_code &error )
{
    if( !error )
    {
        server_conn->begin();
    }

    begin_accept( m_acceptor.get_io_service() );
}

int s_main( int argc, char ** argv )
{
    try
    {
        boost::asio::io_service io;
        PetEscapeServer pes( io );

        // Spin off server operations into another thread.

        io.run();
    }
    catch( std::exception &e )
    {
        std::cerr << e.what() << std::endl;
        return -1;
    }

    return 0;
}

}}}
