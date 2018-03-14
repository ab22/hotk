#include "tcp_client.h"

using namespace hotk::net;

TcpClient::TcpClient(const char* server, const char* port)
    : _server(server)
    , _port(port)
    , _socket(_io_service)
{
}

void TcpClient::connect()
{
    tcp::resolver resolver(_io_service);
    tcp::resolver::query query(_server, _port);
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

    boost::asio::connect(_socket, endpoint_iterator);
}

bool TcpClient::is_connected()
{
    return _socket.is_open();
}

size_t TcpClient::send(std::vector<std::byte>& data)
{
    return boost::asio::write(_socket, boost::asio::buffer(data));
}

size_t TcpClient::send(void* data, size_t len)
{
    auto boost_buffer = boost::asio::buffer(data, len);

    return boost::asio::write(_socket, boost_buffer);
}

void TcpClient::close()
{
    _socket.close();
}
