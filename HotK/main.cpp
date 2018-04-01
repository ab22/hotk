#define _SILENCE_ALL_CXX17_DEPRECATION_WARNINGS

#include <fstream>
#include <iostream>
#include <cstdint>
#include <vector>
#include <cstring>
#include <cstddef>

#include <boost/asio.hpp>

#include "errors/errors.h"
#include "graphics/graphics.h"
#include "net/tcp_client.h"

using hotk::errors::ErrorCode;
using hotk::graphics::Graphics;
using hotk::net::TcpClient;

using tcp = boost::asio::ip::tcp;
using boost::system::error_code;

void on_message_sent(TcpClient &tcp_client, const error_code err, const size_t length)
{
    if (err) {
        std::cout << "Failed to send message:" << err.message() << "\n";
        std::cout << "Bytes written:" << length << "\n";
        return;
    }

    std::cout << length << " bytes sent to server!\n";
}

void on_message_receive(TcpClient &tcp_client, const error_code err, const size_t length)
{
    if (err == boost::asio::error::eof || err == boost::asio::error::connection_reset) {
        std::cout << "Server has ended the connection:" << err.message() << "\n";
        return;
    } else if (err) {
        std::cout << "Error reading message:" << err.message() << "\n";
        return;
    }

    std::cout << "Read message of " << length << "bytes\n";

    try {
        std::cout << "Initializing graphics module...\n";
        Graphics g;

        std::cout << "Capturing Screen...\n";
        auto screen_hbitmap = g.capture_screen();

        std::cout << "Grabbing image data...\n";
        auto bitmap = g.to_vector(screen_hbitmap.get());

        std::vector<std::byte> packet_header;
        uint64_t header = bitmap.size();
        packet_header.resize(sizeof(header));
        std::memcpy(packet_header.data(), &header, sizeof(header));

        tcp_client.send(std::move(packet_header), on_message_sent);
        tcp_client.send(std::move(bitmap), on_message_sent);
    } catch (const ErrorCode& err) {
        std::cout << "Unhandled error caught:\n"
            << "        code: " << err.code() << "\n"
            << "     message: " << err.what() << "\n";
    }
}

void on_connect(TcpClient &tcp_client, const error_code err)
{
    if (err) {
        std::cout << "Error connecting: " << err.message() << "\n";
        return;
    }

    std::cout << "Connected to server!\n";
    std::cout << "Awaiting for message...\n";
    tcp_client.read(on_message_receive);
    std::cout << "You can spam a new async call to write message here :D\n";
}

void connect_to_server()
{
    std::cout << "Connecting to server on port 8080..." << std::endl;
    TcpClient tcp_client("127.0.0.1", "8080");

    tcp_client.connect(on_connect);

    std::thread io_service_thread = std::thread([&tcp_client]() {
        std::cout << "Starting io ctx thread\n";
        tcp_client.run();
        std::cout << "Ending io ctx thread\n";
    });

    system("pause");

    tcp_client.stop();
    io_service_thread.join();
    tcp_client.close();
}

int main()
{
    std::cout << "Starting program...\n";

    try {
        std::cout << "Establishing connection to server..." << "\n";
        connect_to_server();
        std::cout << "Done! Have a good day commander!\n";
    } catch (const std::exception& err) {
        std::cout << "Unhandled Exception caught:\n"
                  << " message:" << err.what() << "\n";
    }

    system("pause");
    return 0;
}
