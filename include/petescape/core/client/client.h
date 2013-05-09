#ifndef CLIENT_H
#define CLIENT_H 1

#include "petescape/networking/client/ClientConnection.h"
#include "petescape/networking/common/net_struct.h"
#include "petescape/core/GameObject.h"
#include "petescape/core/BlockMap.h"

#include <map>
#include <boost/asio.hpp>

namespace petescape {
namespace core {
namespace client {

int c_main( int, char ** );

using boost::asio::ip::tcp;
using namespace petescape::networking::client;

class PetEscapeClient
{
public:
    PetEscapeClient(boost::asio::io_service & , const char *ipAddress);

    inline void setNetworkEventSource( ALLEGRO_EVENT_SOURCE *src )
        { m_client_ptr->setEventSource( src ); }

    void updateObject( const update_obj* );
    void destoryObject( const destroy_obj* );

    void write( packet_list plist, packet_id id );
    void cleanup();

    ClientConnection::client_conn_ptr getConnection(){
        return m_client_ptr;
    }

private:
    void init();

    std::map<uint32_t, GameObject*> m_objects;

    tcp::resolver           m_resolver;
    tcp::resolver::iterator m_end_point;
    uint64_t                m_start_time;
    uint8_t                 m_client_id;
    ClientConnection::client_conn_ptr m_client_ptr;
};

}}}

#endif
