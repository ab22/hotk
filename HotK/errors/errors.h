#pragma once

#include <exception>
#include <string>

namespace hotk::errors {
	class ErrorCode : public std::runtime_error {
	private:
		int _code;

	public:
		ErrorCode() noexcept;
		ErrorCode(int code, const char* message) noexcept;
		ErrorCode(int code, std::string& message) noexcept;
		ErrorCode(ErrorCode&&) = default;

		virtual int code() const noexcept;
		virtual explicit operator bool() const noexcept;
		virtual ErrorCode& operator=(const ErrorCode&) = default;
		virtual ErrorCode& operator=(ErrorCode&&) = default;
	};
}
