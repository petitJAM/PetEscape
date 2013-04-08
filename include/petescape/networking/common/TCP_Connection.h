#ifndef TCP_CONNECTION_H
#define TCP_CONNECTION_H 1

#include <stdint.h>
#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

#include "net_struct.h"

namespace petescape {
namespace networking {
namespace common {

using boost::asio::ip::tcp;

class TCP_Connection
{
public:
    inline tcp::socket& getSocket(){ return m_socket; }
    inline uint32_t     getID(){ return m_id; }

    virtual void begin() = 0;

    virtual void async_write( const packet_list &, packet_id, uint8_t ) = 0;
    virtual void sync_write( const packet_list &, packet_id, uint8_t ) = 0;

    virtual void async_read() = 0;
    virtual void sync_read( packet_list &, packet_id & ) = 0;

protected:
    TCP_Connection( boost::asio::io_service &io_s, uint8_t id )
        : m_socket( io_s ), m_id( id ) {}

    virtual void write_callback( const boost::system::error_code &,
                                 size_t ) = 0;

    virtual void read_callback( const boost::system::error_code &,
                                size_t ) = 0;

    tcp::socket     m_socket;
    uint32_t        m_id;
    network_packet  m_input;
    network_packet  m_output;
};

} // End Common Namespace
} // End Networking Namespace
} // End PetEscape Namespace

#endif
