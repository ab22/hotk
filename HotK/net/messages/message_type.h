#pragma once

#include <cstdint>

namespace hotk::net::messages {
	enum class MessageType : uint16_t {
		screen_capture = 1,
		pc_info,
	};
}