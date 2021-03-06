#include "handlers.h"

namespace handlers = hotk::handlers;
namespace screen   = hotk::graphics::screen;

using handlers::TcpClient;
using handlers::MessageType;
using handlers::Win32Error;


void handlers::process_message(TcpClient& tcp_client, const MessageType msg_type, std::vector<std::byte>&&)
{
	switch (msg_type) {
	case MessageType::MachineInfo:
		hotk::handlers::get_machine_info(tcp_client);
		break;

	case MessageType::ScreenCapture:
		hotk::handlers::capture_screen(tcp_client);
		break;

	case MessageType::ServerShutdown:
		std::cout << "Server is shutting down...\n"
			<< "Should try to reconnect in a few seconds maybe??\n";
		break;

	default:
		std::cout << "Unrecognized message type received: " << static_cast<uint16_t>(msg_type) << "\n";
	}
}

std::vector<std::byte> get_computer_name()
{
	std::vector<std::byte> fqdn;
	DWORD                  size = 0;

	GetComputerNameEx(COMPUTER_NAME_FORMAT::ComputerNameDnsFullyQualified, NULL, &size);
	fqdn.resize(static_cast<std::size_t>(size));

	auto err = GetComputerNameEx(
		COMPUTER_NAME_FORMAT::ComputerNameDnsFullyQualified,
		reinterpret_cast<char *>(fqdn.data()),
		&size);

	if (err == 0)
		throw Win32Error(GetLastError(), "get_computer_name: GetComputerNameEx failed");

	return fqdn;
}

void handlers::get_machine_info(TcpClient& tcp_client)
{
	std::vector<std::byte> machine_name;
	std::cout << "Getting machine information...\n";

	try {
		machine_name = get_computer_name();
	}
	catch (const Win32Error& err) {
		std::cout << "Error getting computer name: "
			<< "    code: " << err.code()
			<< " message: " << err.what();
			return;
	}

	std::cout << "Sending machine name...\n";
	tcp_client.write(MessageType::MachineInfo, std::move(machine_name));
}

void handlers::capture_screen(TcpClient& tcp_client)
{
	std::cout << "Capturing full screen...\n";
	auto screenshot = screen::capture_full_screen();

	std::cout << "Grabbing image data...\n";
	auto image_data = screenshot->to_png();

	tcp_client.write(MessageType::ScreenCapture, std::move(image_data));
}