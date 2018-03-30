#include "tcp_client.h"

using namespace hotk::net;


TcpClient::TcpClient(const char* server, const char* port)
    : _server(server)
    , _port(port)
    , _socket(_io_service)
    , _header_size(0)
{
    tcp::resolver resolver(_io_service);
    tcp::resolver::query query(_server, _port);
    _endpoint = resolver.resolve(query);

    _io_ctx_thread = std::thread([this]() {
        std::cout << "Starting io ctx thread\n";
        boost::asio::executor_work_guard<boost::asio::io_service::executor_type> _work_guard = boost::asio::make_work_guard(_io_service);
        _io_service.run();
        std::cout << "Ending io ctx thread\n";
    });
}

void TcpClient::connect(TcpClient::OnConnectCallback callback)
{
    // boost::asio::connect(_socket, endpoint_iterator);
    boost::asio::async_connect(_socket, _endpoint,
        [this, callback](const boost::system::error_code err, const tcp::endpoint) {
            callback(*this, err);
        }
    );
}

void TcpClient::read(TcpClient::OnReadCallback callback)
{
    boost::asio::async_read(_socket, boost::asio::buffer(&_header_size, sizeof(_header_size)),
        [this, callback](const boost::system::error_code err, const size_t length) {
            callback(*this, err, length);
        }
    );
}

bool TcpClient::is_connected()
{
    return _socket.is_open();
}

void TcpClient::send(TcpClient::ByteVector&& data)
{
    boost::asio::post(_io_service, [this, data]() {
        bool queue_empty = _msg_queue.empty();
        uint64_t header = data.size();

        std::vector<std::byte> header_vector;
        header_vector.resize(sizeof(header));
        std::memcpy(header_vector.data(), &header, sizeof(header));

        _msg_queue.push_back(std::move(header_vector));
        _msg_queue.push_back(std::move(data));

        if (queue_empty)
            perform_send();
    });

    // return boost::asio::write(_socket, boost::asio::buffer(data));
}

void TcpClient::perform_send()
{
    boost::asio::async_write(_socket, boost::asio::buffer(_msg_queue.front().data(), _msg_queue.front().size()),
        [this](boost::system::error_code err, std::size_t length) {
            if (err) {
                std::cout << "An error ocurred when writing to server: " << err.message() << "\n";
                return;
            }

            // Remove current message from queue.
            _msg_queue.pop_front();

            if (!_msg_queue.empty())
                perform_send();
        }
    );
}

void TcpClient::run()
{
    _io_ctx_thread.join();
}

void TcpClient::close()
{
    _socket.close();
}

void TcpClient::stop()
{
    _io_service.stop();
}