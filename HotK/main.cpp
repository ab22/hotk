#define _SILENCE_CXX17_RESULT_OF_DEPRECATION_WARNING
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

using hotk::errors::ErrorCode;
using hotk::graphics::Graphics;
using boost::asio::ip::tcp;

void connect_to_server(std::vector<std::byte>& bitmap)
{
    try {
        boost::asio::io_service io_service;
        tcp::resolver resolver(io_service);
        tcp::resolver::query query("127.0.0.1", "8080");
        tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
        tcp::socket socket(io_service);

        std::cout << "Connecting to server on port 8080..." << std::endl;

		boost::asio::connect(socket, endpoint_iterator);

        uint64_t bitmap_size = (uint64_t)bitmap.size();
        std::vector<unsigned char> buffer(sizeof(bitmap_size));
        std::memcpy(buffer.data(), &bitmap_size, sizeof(bitmap_size));

		size_t bytes_sent = boost::asio::write(socket, boost::asio::buffer(buffer));
        std::cout << "Sent " << bytes_sent << " bytes as header to server.\n";

        bytes_sent = boost::asio::write(socket, boost::asio::buffer(bitmap));
        std::cout << "Sent " << bytes_sent << " bytes as bitmap data to server.\n";

        socket.close();
    } catch (std::exception& e) {
        std::cerr << "Asio Exception thrown:\n"
                  << e.what() << std::endl;
    }
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

        // std::cout << "Saving to file..\n";
        // g.save_bitmap_to_file(L"Screenshot.bmp", screen_hbitmap.get());
        // std::ofstream file("Screenshot.bmp", std::ios::binary);

        // if (!file.is_open()) {
        //    std::cout << "Error: could not create file!\n";
        //    return 0;
        // }

        // file.write((const char*)bitmap.data(), bitmap.size());

        std::cout << "Establishing connection to server..." << "\n";
        connect_to_server(bitmap);

        std::cout << "Done! Have a good day commander!\n";
    } catch (const ErrorCode& err) {
        std::cout << "Unhandled error caught:\n"
                  << "        code: " << err.code() << "\n"
                  << "     message: " << err.what() << "\n";
    }

    system("pause");
    return 0;
}
