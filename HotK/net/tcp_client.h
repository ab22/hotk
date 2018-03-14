#pragma once

#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS
#include <boost/asio.hpp>

#include <vector>
#include <cstdint>


namespace hotk::net {
    class TcpClient {
        private:
            using tcp = boost::asio::ip::tcp;
            using io_service = boost::asio::io_service;

            const char* _server;
            const char* _port;
            boost::asio::io_service _io_service;
            tcp::socket _socket;

        public:
            TcpClient(const char* server, const char* port);

            void connect();
            bool is_connected();

            size_t send(std::vector<std::byte>&);
            size_t send(void* data, size_t len);

            void close();
    };
}
