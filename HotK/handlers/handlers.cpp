#include "handlers.h"

namespace handlers = hotk::handlers;

using handlers::TcpClient;
using handlers::MessageType;
using handlers::Win32Error;
using handlers::Graphics;


void handlers::process_message(TcpClient& tcp_client, const MessageType msg_type, std::vector<std::byte>&& data)
{
	switch (msg_type) {
	case MessageType::MachineInfo:
		hotk::handlers::get_machine_info(tcp_client);
		break;

	case MessageType::ScreenCapture:
		hotk::handlers::capture_screen(tcp_client);
		break;

	default:
		std::cout << "Unrecognized message type received: " << (uint16_t)msg_type << "\n";
	}
}

std::string get_computer_name()
{
	std::string fqdn;
	DWORD       size = 0;

	GetComputerNameEx(COMPUTER_NAME_FORMAT::ComputerNameDnsFullyQualified, NULL, &size);
	fqdn.resize((std::size_t)size);

	auto err = GetComputerNameEx(COMPUTER_NAME_FORMAT::ComputerNameDnsFullyQualified, fqdn.data(), &size);

	if (err == 0)
		throw Win32Error(GetLastError(), "get_computer_name: GetComputerNameEx failed");

	return fqdn;
}

void handlers::get_machine_info(TcpClient& tcp_client)
{
	std::string machine_name;
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

	std::vector<std::byte> buffer(
		(std::byte *)machine_name.data(),
		(std::byte *)(machine_name.data() + machine_name.size())
	);
	tcp_client.write(MessageType::MachineInfo, std::move(buffer));
}

void handlers::capture_screen(TcpClient& tcp_client)
{
	std::cout << "Initializing graphics module...\n";
	Graphics g;

	std::cout << "Capturing Screen...\n";
	auto screen_hbitmap = g.capture_screen();

	std::cout << "Grabbing image data...\n";
	auto bitmap = g.to_vector(screen_hbitmap.get());

	tcp_client.write(MessageType::ScreenCapture, std::move(bitmap));
}