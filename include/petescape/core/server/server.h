#ifndef SERVER_H
#define SERVER_H 1

#include <petescape/networking/server/ServerConnection.h>

#include <boost/asio.hpp>

using petescape::networking::server::ServerConnection;

namespace petescape
{
namespace core
{
namespace server
{

int s_main( int, char ** );

using boost::asio::ip::tcp;

class PetEscapeServer
{
public:
    PetEscapeServer( boost::asio::io_service & );

private:
    void begin_accept(boost::asio::io_service &io);
    void handle_accept( ServerConnection::server_conn_ptr,
                        const boost::system::error_code & );

    tcp::acceptor   m_acceptor;
    uint64_t        m_start_time;
};

}
}
}

#endif
