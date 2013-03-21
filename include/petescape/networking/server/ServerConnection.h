#ifndef SERVER_CONNECTION_H
#define SERVER_CONNECTION_H 1

#include <stdint.h>
#include <boost/bind.hpp>
#include <boost/asio.hpp>

#include "../common/net_struct.h"
#include "../common/TCP_Connection.h"

using namespace petescape::networking;

namespace petescape
{
namespace networking
{
namespace server
{

using boost::asio::ip::tcp;

class ServerConnection :
        public common::TCP_Connection,
        public boost::enable_shared_from_this< ServerConnection >
{
public:
    typedef boost::shared_ptr< ServerConnection > server_conn_ptr;

    static inline server_conn_ptr CreateConnection( boost::asio::io_service &io_s,
                                                    const uint32_t &id )
    {
        return server_conn_ptr( new ServerConnection( io_s, id ) );
    }

    void begin();

protected:
    ServerConnection( boost::asio::io_service &io_s, uint8_t id )
        : common::TCP_Connection( io_s, id ) {}

    void write_callback( const boost::system::error_code &,
                         size_t );

    void read_callback( const boost::system::error_code &,
                        size_t );
};

} // End Server Namespace
} // End Networking Namespace
} // End PetEscape Namespace

#endif // SERVER_CONNECTION_H
