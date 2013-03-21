#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H 1

#include <stdint.h>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

#include "net_struct.h"

namespace petescape
{
namespace networking
{
namespace common
{
using boost::asio::ip::tcp;

class TCP_Connection /*: public
        boost::enable_shared_from_this< TCP_Connection >*/
{
public:
    tcp::socket& getSocket();

    virtual void begin() = 0;

protected:
    TCP_Connection( boost::asio::io_service &io_s, uint8_t id )
        : m_socket( io_s ), m_id( id ) {}

    virtual void write_callback( const boost::system::error_code &,
                                 size_t ) = 0;

    virtual void read_callback( const boost::system::error_code &,
                                size_t ) = 0;

    tcp::socket m_socket;
    uint8_t     m_id;
    packet_list m_input;
    packet_list m_output;
};

} // End Common Namespace
} // End Networking Namespace
} // End PetEscape Namespace

#endif
