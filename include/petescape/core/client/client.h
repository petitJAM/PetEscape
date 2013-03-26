#ifndef CLIENT_H
#define CLIENT_H 1

#include <boost/asio.hpp>

namespace petescape
{
namespace core
{
namespace client
{

int c_main( int, char ** );

using boost::asio::ip::tcp;

class PetEscapeClient
{
public:
    PetEscapeClient( boost::asio::io_service & , char* ipAddress);

private:
    void init();

    tcp::resolver   m_resolver;
    uint64_t        m_start_time;
    uint8_t         m_client_id;
};

}
}
}
#endif
