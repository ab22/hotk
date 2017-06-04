#pragma once

#include <string>
#include "../errors/errors.h"

namespace hotk {
	namespace graphics {
		class GDIStartupError : public hotk::errors::ErrorCode {
		public:
			GDIStartupError() noexcept;
			GDIStartupError(int code, const char* message) noexcept;
			GDIStartupError(int code, std::string& message) noexcept;
		};

		class EncoderClsIDError : public hotk::errors::ErrorCode {
		public:
			EncoderClsIDError() noexcept;
			EncoderClsIDError(int code, const char* message) noexcept;
			EncoderClsIDError(int code, std::string& message) noexcept;
		};

		class CaptureScreenError : public hotk::errors::ErrorCode {
		public:
			CaptureScreenError() noexcept;
			CaptureScreenError(int code, const char* message) noexcept;
			CaptureScreenError(int code, std::string& message) noexcept;
		};
	}
}