
#include "petescape/networking/common/TCP_Connection.h"
#include "petescape/networking/server/ServerConnection.h"
#include "petescape/networking/client/ClientConnection.h"

using petescape::networking::common::TCP_Connection;
using namespace boost::asio::ip;

tcp::socket& TCP_Connection::getSocket()
{
    return this->m_socket;
}
