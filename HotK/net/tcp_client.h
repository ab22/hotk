#pragma once

#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
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
            using OnConnectCallback = void(*)(TcpClient&, const boost::system::error_code);
            using OnReadCallback = void(*)(TcpClient&, const boost::system::error_code, const size_t);
            using OnWriteCallback = void(*)(TcpClient&, const boost::system::error_code, const size_t);
            using ByteVector = std::vector<std::byte>;

            const char* _server;
            const char* _port;
            boost::asio::io_context _io_service;
            tcp::socket _socket;
            tcp::resolver::results_type _endpoint;
            std::deque<ByteVector> _msg_queue;

            OnConnectCallback on_connect;
            OnReadCallback on_read;
            OnWriteCallback on_write;

            uint64_t _header_size;

            void perform_write();

        public:
            TcpClient(const char* server, const char* port, OnConnectCallback on_connect, OnReadCallback on_read, OnWriteCallback on_write);

            void connect();
            void read();
            bool is_connected();

            void write(ByteVector&&);

            void stop();
            void run();
            void close();
    };
}
