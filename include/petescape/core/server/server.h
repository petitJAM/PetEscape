#ifndef SERVER_H
#define SERVER_H 1

#include "petescape/networking/server/ServerConnection.h"
#include "petescape/networking/common/net_struct.h"
#include "petescape/core/GameObject.h"

#include <map>
#include <boost/asio.hpp>

using petescape::networking::server::ServerConnection;

namespace petescape {
namespace core {
namespace server {

int s_main( int, char ** );

using boost::asio::ip::tcp;

class PetEscapeServer
{
public:
    PetEscapeServer( boost::asio::io_service & );

    inline void setNetworkEventSource( ALLEGRO_EVENT_SOURCE *src )
    { m_server_ptr->setEventSource( src ); }

    void createObject();
    void destroyObject( uint32_t id );
    std::map<uint32_t,GameObject*>* getObjects() {
        return &m_objects;
    }

    ServerConnection::server_conn_ptr getConnection() {
        return m_server_ptr;
    }

private:
    void begin_accept(boost::asio::io_service &io);
    void handle_accept( ServerConnection::server_conn_ptr,
                        const boost::system::error_code & );

    std::map<uint32_t, GameObject *>  m_objects;
    ServerConnection::server_conn_ptr m_server_ptr;
    tcp::acceptor   m_acceptor;
    uint64_t        m_start_time;
};

}}}

#endif
