#pragma once

#include <boost/asio.hpp>

#include <vector>
#include <cstdint>
#include <deque>
#include <iostream>


namespace hotk::net {
    class TcpClient {
        private:
            using tcp = boost::asio::ip::tcp;
            using io_context = boost::asio::io_context;
            using const_buffer = boost::asio::const_buffer;
            using ByteVector = std::vector<std::byte>;

            using OnConnectCallback = void(*)(TcpClient&, const boost::system::error_code);
            using OnReadCallback = void(*)(TcpClient&, const boost::system::error_code, ByteVector&&);
            using OnWriteCallback = void(*)(TcpClient&, const boost::system::error_code, const size_t);

            const char* _server;
            const char* _port;
            boost::asio::io_context _io_service;
            tcp::socket _socket;
            tcp::resolver::results_type _endpoint;
            std::deque<const_buffer> _msg_queue;

            OnConnectCallback on_connect;
            OnReadCallback on_read;
            OnWriteCallback on_write;

            uint64_t _packet_size;
            ByteVector _internal_read_buffer;

            void perform_write();

        public:
            TcpClient(const char* server, const char* port, OnConnectCallback on_connect, OnReadCallback on_read, OnWriteCallback on_write);

            void connect();
            void read();
            void read_data(uint64_t packet_size);
            bool is_connected() const;

            void write(ByteVector&&);

            void stop();
            void run();
            void close();
    };
}
