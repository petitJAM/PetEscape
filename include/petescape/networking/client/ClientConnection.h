#ifndef CLIENT_CONNECTION_H
#define CLIENT_CONNECTION_H 1

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
namespace client
{

using boost::asio::ip::tcp;

class ClientConnection :
        public common::TCP_Connection,
        public boost::enable_shared_from_this< ClientConnection >
{
public:
    typedef boost::shared_ptr< ClientConnection > client_conn_ptr;

    static inline client_conn_ptr CreateConnection( boost::asio::io_service &io_s,
                                                    const uint32_t &id )
    {
        return client_conn_ptr( new ClientConnection( io_s, id ) );
    }

    void begin();

protected:
    ClientConnection( boost::asio::io_service &io_s, uint8_t id )
        : common::TCP_Connection( io_s, id ) {}

    void write_callback( const boost::system::error_code &,
                         size_t );

    void read_callback( const boost::system::error_code &,
                        size_t );
};

} // End Client Namespace
} // End Networking Namespace
} // End PetEscape Namespace

#endif // CLIENT_CONNECTION_H
