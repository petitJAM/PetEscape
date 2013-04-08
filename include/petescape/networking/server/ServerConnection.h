#ifndef SERVER_CONNECTION_H
#define SERVER_CONNECTION_H 1

#include <stdint.h>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <allegro5/allegro.h>

#include "../common/net_struct.h"
#include "../common/TCP_Connection.h"

using namespace petescape::networking::common;

namespace petescape {
namespace networking {
namespace server {

using boost::asio::ip::tcp;

class ServerConnection :
        public common::TCP_Connection,
        public boost::enable_shared_from_this< ServerConnection >
{
    uint64_t m_server_start_time;

public:
    typedef boost::shared_ptr< ServerConnection > server_conn_ptr;

    static inline server_conn_ptr CreateConnection( boost::asio::io_service &io_s,
                                                    const uint32_t &id )
    {
        return server_conn_ptr( new ServerConnection( io_s, id ) );
    }

    void begin();

    void setStartTime( time_t t ){ this->m_server_start_time = t; }

    void async_write( const packet_list &, packet_id, uint8_t rr = 0 );
    void sync_write( const packet_list &, packet_id, uint8_t rr = 0 );

    void async_read();
    void sync_read();
    void sync_read( packet_list &, packet_id & );

    void setEventSource( ALLEGRO_EVENT_SOURCE *src );

protected:
    ServerConnection( boost::asio::io_service &io_s, uint32_t id )
        : common::TCP_Connection( io_s, id ) {
        this->m_event_dispatcher = nullptr;
    }

    void write_callback( const boost::system::error_code &,
                         size_t );

    void read_callback( const boost::system::error_code &,
                        size_t );

    inline uint8_t GenClientID()
    {
        static uint8_t id = 0;
        return ++id;
    }

    void handle_client_connect();

    ALLEGRO_EVENT_SOURCE *m_event_dispatcher;
};

} // End Server Namespace
} // End Networking Namespace
} // End PetEscape Namespace

#endif // SERVER_CONNECTION_H
