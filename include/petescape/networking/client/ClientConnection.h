#ifndef CLIENT_CONNECTION_H
#define CLIENT_CONNECTION_H 1

#include <stdint.h>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <allegro5/allegro.h>

#include "../common/net_struct.h"
#include "../common/TCP_Connection.h"

namespace petescape {
namespace networking {
namespace client {

using boost::asio::ip::tcp;
using namespace petescape::networking::common;

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

    void sync_read( packet_list &, packet_id & );
    void sync_write( const packet_list &, packet_id , uint8_t rr = 0);

    void async_read( packet_list &, packet_id & ){}
    void async_write( const packet_list &, packet_id, uint8_t rr = 0 ){}

    inline void setID( const uint32_t id ){ this->m_id = id; }

    void setEventSource( ALLEGRO_EVENT_SOURCE *src );

    void begin();

protected:
    ClientConnection( boost::asio::io_service &io_s, uint32_t id );

    void write_callback( const boost::system::error_code &,
                         size_t );

    void read_callback( const boost::system::error_code &,
                        size_t );

    ALLEGRO_EVENT_SOURCE *m_event_dispatcher;
};

} // End Client Namespace
} // End Networking Namespace
} // End PetEscape Namespace

#endif // CLIENT_CONNECTION_H
