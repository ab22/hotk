#pragma once

#include <iostream>
#include <vector>

#include "../net/messages/message_type.h"
#include "../net/tcp_client.h"
#include "../graphics/graphics.h"
#include "../winutils/errors.h"


namespace hotk::handlers {
	using TcpClient = hotk::net::TcpClient;
	using MessageType = hotk::net::messages::MessageType;
	using Win32Error = hotk::winutils::errors::Win32Error;
	using Graphics = hotk::graphics::Graphics;

	void get_machine_info(TcpClient&);
	void capture_screen(TcpClient&);

	void process_message(TcpClient&, const MessageType, std::vector<std::byte>&&);
}
