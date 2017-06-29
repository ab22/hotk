#pragma once

#include <string>
#include "../errors/errors.h"

namespace hotk::graphics {
	class GraphicsInitError : public hotk::errors::ErrorCode {
	public:
		GraphicsInitError() noexcept;
		GraphicsInitError(int code, const char* message) noexcept;
		GraphicsInitError(int code, std::string& message) noexcept;
	};

	class CaptureScreenError : public hotk::errors::ErrorCode {
	public:
		CaptureScreenError() noexcept;
		CaptureScreenError(int code, const char* message) noexcept;
		CaptureScreenError(int code, std::string& message) noexcept;
	};
}