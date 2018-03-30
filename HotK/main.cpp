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

void on_message(TcpClient &tcp_client, const boost::system::error_code err, const size_t length)
{
    if (err) {
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

        tcp_client.send(std::move(bitmap));
    } catch (const ErrorCode& err) {
        std::cout << "Unhandled error caught:\n"
            << "        code: " << err.code() << "\n"
            << "     message: " << err.what() << "\n";
    }
}

void on_connect(TcpClient &tcp_client, const boost::system::error_code err)
{
    if (err) {
        std::cout << "Error connecting: " << err.message() << "\n";
        return;
    }

    std::cout << "Connected to server!\n";
    std::cout << "Awaiting for message...\n";
    tcp_client.read(on_message);
    std::cout << "You can spam a new async call to write message here :D\n";
}

void connect_to_server()
{
    std::cout << "Connecting to server on port 8080..." << std::endl;
    TcpClient tcp_client("127.0.0.1", "8080");

    tcp_client.connect(on_connect);

    system("pause");
    tcp_client.stop();
    tcp_client.run();
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
