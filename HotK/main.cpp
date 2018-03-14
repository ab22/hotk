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

void connect_to_server(std::vector<std::byte>& bitmap)
{
    std::cout << "Connecting to server on port 8080..." << std::endl;
    TcpClient tcp_client("127.0.0.1", "8080");
    tcp_client.connect();

    uint64_t bitmap_size = bitmap.size();
    size_t bytes_sent = 0;

    bytes_sent = tcp_client.send((void*)&bitmap_size, sizeof(uint64_t));
    std::cout << "Sent " << bytes_sent << " bytes as header to server.\n";

    bytes_sent = tcp_client.send(bitmap);
    std::cout << "Sent " << bytes_sent << " bytes as header to server.\n";

    std::cout << "Closing connection...\n";
	tcp_client.close();
}

int main()
{
    std::cout << "Starting program...\n";

    try {
        std::cout << "Initializing graphics module...\n";
        Graphics g;

        std::cout << "Capturing Screen...\n";
        auto screen_hbitmap = g.capture_screen();

        std::cout << "Grabbing image data...\n";
        auto bitmap = g.to_vector(screen_hbitmap.get());

        std::cout << "Establishing connection to server..." << "\n";
        connect_to_server(bitmap);

        std::cout << "Done! Have a good day commander!\n";
    } catch (const ErrorCode& err) {
        std::cout << "Unhandled error caught:\n"
                  << "        code: " << err.code() << "\n"
                  << "     message: " << err.what() << "\n";
    } catch (const std::exception& err) {
        std::cout << "Unhandled Exception caught:\n"
                  << " message:" << err.what() << "\n";
    }

    system("pause");
    return 0;
}
