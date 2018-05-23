#include "tcp_client.h"

using namespace hotk::net;

using boost::asio::buffer;
using boost::system::error_code;

using boost::asio::executor_work_guard;
using boost::asio::make_work_guard;
using executor_type = boost::asio::io_service::executor_type;

using hotk::net::containers::BaseContainer;
using hotk::net::containers::PtrContainer;
using hotk::net::containers::VectorContainer;
using hotk::net::containers::PrimitiveContainer;

TcpClient::TcpClient(const char* server, const char* port, OnConnectCallback on_connect,
		OnReadCallback on_read, OnWriteCallback on_write)
	: _server(server)
	, _port(port)
	, _socket(_io_service)
	, _packet_size(0)
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
	// Clear buffer in case we left it in an undefined state.
	_internal_read_buffer.clear();

	// Read the packet size of the message.
	boost::asio::async_read(_socket, buffer(&_packet_size, sizeof(_packet_size)),
		[this](const error_code err, const size_t) {
			if (err) {
				on_read(*this, err, (TcpClient::MessageType)0, std::move(_internal_read_buffer));
				return;
			}

			read_msg_type(_packet_size);
		}
	);
}

void TcpClient::read_msg_type(uint64_t packet_size)
{
	// Read the message type.
	boost::asio::async_read(_socket, buffer(&_message_type, sizeof(_message_type)),
		[this, packet_size](const error_code err, const size_t) {
			if (err) {
				on_read(*this, err, (TcpClient::MessageType)0, std::move(_internal_read_buffer));
				return;
			}

			// If request does not have any data.
			if (packet_size == 0) {
				on_read(*this, err, _message_type, std::move(_internal_read_buffer));
				return;
			}

			read_data(packet_size, _message_type);
		}
	);
}

void TcpClient::read_data(uint64_t packet_size, MessageType msg_type)
{
	_internal_read_buffer.resize(packet_size);

	// Read the actual data from the server.
	boost::asio::async_read(_socket, buffer(_internal_read_buffer),
		[this, msg_type](const error_code err, const size_t) {
			on_read(*this, err, msg_type, std::move(_internal_read_buffer));
		}
	);
}

bool TcpClient::is_connected() const
{
	return _socket.is_open();
}

void TcpClient::write(TcpClient::MessageType msg_type, const char* data, std::size_t size)
{
	boost::asio::post(_io_service, [this, msg_type, data, size]() mutable {
		bool queue_empty = _msg_queue.empty();

		// Send first the size of the packet as a uint64_t.
		auto header_container = new PrimitiveContainer<uint64_t>(size);
		_msg_queue.push_back(std::unique_ptr<BaseContainer>(header_container));

		auto msg_type_container = new PrimitiveContainer<uint16_t>((uint16_t)msg_type);
		_msg_queue.push_back(std::unique_ptr<BaseContainer>(msg_type_container));

		auto data_container = new PtrContainer(data, size);
		_msg_queue.push_back(std::unique_ptr<BaseContainer>(data_container));

		if (queue_empty)
			perform_write();
	});
}

void TcpClient::write(TcpClient::MessageType msg_type, TcpClient::ByteVector&& data)
{
	boost::asio::post(_io_service, [this, msg_type, data]() {
		bool queue_empty = _msg_queue.empty();

		// Send first the size of the packet as a uint64_t.
		auto header_container = new PrimitiveContainer<uint64_t>(data.size());
		_msg_queue.push_back(std::unique_ptr<BaseContainer>(header_container));

		auto msg_type_container = new PrimitiveContainer<uint16_t>((uint16_t)msg_type);
		_msg_queue.push_back(std::unique_ptr<BaseContainer>(msg_type_container));

		auto data_container = new VectorContainer<std::byte>(std::move(data));
		_msg_queue.push_back(std::unique_ptr<BaseContainer>(data_container));

		if (queue_empty)
			perform_write();
	});
}

void TcpClient::perform_write()
{
	auto& next_message = _msg_queue.front();
	const char* data = next_message.get()->data();
	std::size_t size = next_message.get()->size();

	boost::asio::async_write(_socket, buffer(data, size),
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
	// Remove all pending messages on disconnect/close.
	_msg_queue.clear();
	_socket.close();
}

void TcpClient::stop()
{
	_io_service.stop();
}