
#include "petescape/core/client/client.h"
#include "petescape/networking/client/ClientConnection.h"
#include "petescape/networking/common/net_struct.h"

#include <boost/bind.hpp>
#include <boost/asio.hpp>

namespace petescape {
namespace core {
namespace client {

using petescape::networking::client::ClientConnection;

PetEscapeClient::PetEscapeClient(boost::asio::io_service &io, const char* ipAddress) :
    m_resolver( io )
{
#ifdef DEBUG
    std::cout << "Here?\n";
    std::cout << ipAddress << "\n";
#endif
    tcp::resolver::query q( ipAddress, "2001" );
    tcp::resolver::iterator end_point = m_resolver.resolve( q );

    ClientConnection::client_conn_ptr client_ptr =
            ClientConnection::CreateConnection( io, 0 );

    boost::asio::connect( client_ptr->getSocket(), end_point );
#ifdef DEBUG
    std::cout << "Done with init stuff, handshake time," << std::endl;
#endif
    // Run through handshake with server.
    client_ptr->begin();
}

int c_main( int argc, char **argv )
{
    try
    {
        boost::asio::io_service io;
        PetEscapeClient pec( io, argv[2]);

        // At this point, we need to kick into a thread
        // to do Allegro stuff.

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
