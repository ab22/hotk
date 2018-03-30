#pragma once

#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#include <boost/asio.hpp>

#include <vector>
#include <cstdint>
#include <thread>
#include <iostream>
#include <deque>


#include "../graphics/graphics.h"

namespace hotk::net {
    class TcpClient {
        private:
            using tcp = boost::asio::ip::tcp;
            using io_context = boost::asio::io_context;
            using OnConnectCallback = void(*)(TcpClient&, const boost::system::error_code);
            using OnReadCallback = void(*)(TcpClient&, const boost::system::error_code, const size_t);
            using OnWriteCallback = void(*)(TcpClient&);
            using ByteVector = std::vector<std::byte>;

            const char* _server;
            const char* _port;
            boost::asio::io_context _io_service;
            tcp::socket _socket;
            tcp::resolver::results_type _endpoint;
            std::thread _io_ctx_thread;
            std::deque<ByteVector> _msg_queue;

            uint64_t _header_size;

            void perform_send();

        public:
            TcpClient(const char* server, const char* port);

            void connect(OnConnectCallback);
            void read(OnReadCallback);
            bool is_connected();

            void send(ByteVector&&);

            void stop();
            void run();
            void close();
    };
}
