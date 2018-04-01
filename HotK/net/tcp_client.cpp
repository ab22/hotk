#include "tcp_client.h"

using namespace hotk::net;

using boost::asio::buffer;
using boost::system::error_code;

using boost::asio::executor_work_guard;
using boost::asio::make_work_guard;
using executor_type = boost::asio::io_service::executor_type;

TcpClient::TcpClient(const char* server, const char* port)
    : _server(server)
    , _port(port)
    , _socket(_io_service)
    , _header_size(0)
{
    tcp::resolver resolver(_io_service);
    tcp::resolver::query query(_server, _port);
    _endpoint = resolver.resolve(query);
}

void TcpClient::connect(const TcpClient::OnConnectCallback callback)
{
    boost::asio::async_connect(_socket, _endpoint,
        [this, callback](const error_code err, const tcp::endpoint) {
            callback(*this, err);
        }
    );
}

void TcpClient::read(const TcpClient::OnReadCallback callback)
{
    boost::asio::async_read(_socket, buffer(&_header_size, sizeof(_header_size)),
        [this, callback](const error_code err, const size_t length) {
            callback(*this, err, length);
        }
    );
}

bool TcpClient::is_connected()
{
    return _socket.is_open();
}

void TcpClient::send(TcpClient::ByteVector&& data, const TcpClient::OnWriteCallback callback)
{
    boost::asio::post(_io_service, [this, data]() {
        bool queue_empty = _msg_queue.empty();
        _msg_queue.push_back(std::move(data));

        if (queue_empty)
            perform_send();
    });
}

void TcpClient::perform_send()
{
    boost::asio::async_write(_socket, buffer(_msg_queue.front().data(), _msg_queue.front().size()),
        [this](error_code err, std::size_t length) {
            if (err == boost::asio::error::eof || err == boost::asio::error::connection_reset) {
                std::cout << "Server has ended the connection:" << err.message() << "\n";
                return;
            } else if (err) {
                std::cout << "An error ocurred when writing to server: " << err.message() << "\n";
                return;
            }

            std::cout << length << " bytes sent to server!\n";

            // Remove current message from queue.
            _msg_queue.pop_front();

            if (!_msg_queue.empty())
                perform_send();
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