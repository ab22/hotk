#include "tcp_client.h"

using namespace hotk::net;

using boost::asio::buffer;
using boost::system::error_code;

using boost::asio::executor_work_guard;
using boost::asio::make_work_guard;
using executor_type = boost::asio::io_service::executor_type;

TcpClient::TcpClient(const char* server, const char* port, OnConnectCallback on_connect, OnReadCallback on_read, OnWriteCallback on_write)
    : _server(server)
    , _port(port)
    , _socket(_io_service)
    , _header_size(0)
    , on_connect(on_connect)
    , on_read(on_read)
    , on_write(on_write)
{
    tcp::resolver resolver(_io_service);
    tcp::resolver::query query(_server, _port);
    _endpoint = resolver.resolve(query);
}

void TcpClient::connect()
{
    boost::asio::async_connect(_socket, _endpoint,
        [this](const error_code err, const tcp::endpoint) {
            on_connect(*this, err);
        }
    );
}

void TcpClient::read()
{
    boost::asio::async_read(_socket, buffer(&_header_size, sizeof(_header_size)),
        [this](const error_code err, const size_t length) {
            on_read(*this, err, length);
        }
    );
}

bool TcpClient::is_connected()
{
    return _socket.is_open();
}

void TcpClient::write(TcpClient::ByteVector&& data)
{
    boost::asio::post(_io_service, [this, data]() {
        bool queue_empty = _msg_queue.empty();
        _msg_queue.push_back(std::move(data));

        if (queue_empty)
            perform_write();
    });
}

void TcpClient::perform_write()
{
    auto& next_message = _msg_queue.front();

    boost::asio::async_write(_socket, buffer(next_message),
        [this](error_code err, std::size_t length) {
            if (err) {
                on_write(*this, err, length);
                return;
            }

            // Remove current message from queue.
            _msg_queue.pop_front();

            if (!_msg_queue.empty())
                perform_write();

            on_write(*this, err, length);
        }
    );
}

void TcpClient::run()
{
    executor_work_guard<executor_type> _work_guard = make_work_guard(_io_service);
    _io_service.run();
}

void TcpClient::close()
{
    _socket.close();
}

void TcpClient::stop()
{
    _io_service.stop();
}