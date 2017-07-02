#pragma once

#include <string>
#include "../errors/errors.h"

namespace hotk::winutils::errors {
	class Win32Error : public hotk::errors::ErrorCode {
	public:
		Win32Error() noexcept;
		Win32Error(int code, const char* message) noexcept;
		Win32Error(int code, std::string& message) noexcept;
	};
}