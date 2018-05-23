#pragma once

#include <cstdint>

namespace hotk::net::messages {
	enum class MessageType : uint16_t {
		ScreenCapture = 1,
		MachineInfo
	};
}