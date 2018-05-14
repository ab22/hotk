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
	// Read the packet size of the message.
	boost::asio::async_read(_socket, buffer(&_packet_size, sizeof(_packet_size)),
		[this](const error_code err, const size_t) {
		if (err) {
			// Clear buffer in case we left it in an undefined state.
			_internal_read_buffer.clear();

			on_read(*this, err, std::move(_internal_read_buffer));
			return;
		}

		std::cout << "[TCPClient]: read packet size of " << _packet_size << "\n";
		read_data(_packet_size);
	}
	);
}

void TcpClient::read_data(uint64_t packet_size)
{
	_internal_read_buffer.clear();
	_internal_read_buffer.resize(packet_size);

	// Read the actual data from the server.
	boost::asio::async_read(_socket, buffer(_internal_read_buffer),
		[this](const error_code err, const size_t) {
		uint64_t message_code = *((uint64_t*)_internal_read_buffer.data());
		std::cout << "[TCPClient]: read " << _internal_read_buffer.size() << " bytes\n";
		std::cout << "[TCPClient]: message code is: " << message_code << "\n";
		on_read(*this, err, std::move(_internal_read_buffer));
	}
	);
}

bool TcpClient::is_connected() const
{
	return _socket.is_open();
}

void TcpClient::write(const char* data, std::size_t size)
{
	boost::asio::post(_io_service, [this, data, size]() mutable {
		bool queue_empty = _msg_queue.empty();

		// Send first the size of the packet as a uint64_t.
		uint64_t packet_size = (uint64_t)size;
		std::vector<std::byte> header_packet(sizeof(packet_size));
		std::memcpy(header_packet.data(), &packet_size, sizeof(packet_size));

		auto header_container = new VectorContainer<std::byte>(std::move(header_packet));
		_msg_queue.push_back(std::unique_ptr<BaseContainer>(header_container));

		auto data_container = new PtrContainer(data, size);
		_msg_queue.push_back(std::unique_ptr<BaseContainer>(data_container));

		if (queue_empty)
			perform_write();
	});
}

void TcpClient::write(TcpClient::ByteVector&& data)
{
	boost::asio::post(_io_service, [this, data]() {
		bool queue_empty = _msg_queue.empty();

		// Send first the size of the packet as a uint64_t.
		uint64_t packet_size = data.size();
		std::vector<std::byte> header_packet(sizeof(packet_size));
		std::memcpy(header_packet.data(), &packet_size, sizeof(packet_size));

		auto header_container = new VectorContainer<std::byte>(std::move(header_packet));
		_msg_queue.push_back(std::unique_ptr<BaseContainer>(header_container));

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
	_socket.close();
}

void TcpClient::stop()
{
	_io_service.stop();
}