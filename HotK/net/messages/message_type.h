#pragma once

#include <cstdint>

namespace hotk::net::messages {
	enum class MessageType : uint16_t {
		None = 0,
		ScreenCapture,
		MachineInfo,
		ServerShutdown,
	};
}