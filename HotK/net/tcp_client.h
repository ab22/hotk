#pragma once

#include <boost/asio.hpp>

#include <vector>
#include <cstdint>
#include <deque>
#include <iostream>

#include "containers/message_containers.h"
#include "messages/message_type.h"

namespace hotk::net {
	class TcpClient {
	private:
		using tcp = boost::asio::ip::tcp;
		using io_context = boost::asio::io_context;
		using BaseContainer = hotk::net::containers::BaseContainer;
		using MessageType = hotk::net::messages::MessageType;
		using ByteVector = std::vector<std::byte>;

		using OnConnectCallback = void(*)(TcpClient&, const boost::system::error_code);
		using OnReadCallback = void(*)(TcpClient&, const boost::system::error_code, const MessageType, ByteVector&&);
		using OnWriteCallback = void(*)(TcpClient&, const boost::system::error_code, const size_t);

		const char* _server;
		const char* _port;
		boost::asio::io_context _io_service;
		tcp::socket _socket;
		tcp::resolver::results_type _endpoint;
		std::deque< std::unique_ptr<BaseContainer> > _msg_queue;

		OnConnectCallback on_connect;
		OnReadCallback on_read;
		OnWriteCallback on_write;

		uint64_t _packet_size;
		MessageType _message_type;
		ByteVector _internal_read_buffer;

		void perform_write();

		void read_msg_type(uint64_t);
		void read_data(uint64_t, MessageType);

	public:
		TcpClient(const char* server, const char* port, OnConnectCallback on_connect, OnReadCallback on_read, OnWriteCallback on_write);

		void connect();
		void read();
		bool is_connected() const;

		void write(MessageType, const char*, std::size_t);
		void write(MessageType, ByteVector&&);

		void stop();
		void run();
		void close();
		void clear_msg_queue();
	};
}
