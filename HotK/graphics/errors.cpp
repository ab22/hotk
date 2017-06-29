#include "errors.h"

using namespace hotk::graphics;

#pragma region GraphicsInitError

GraphicsInitError::GraphicsInitError() noexcept
	: ErrorCode()
{

}

GraphicsInitError::GraphicsInitError(int code, const char* message) noexcept
	: ErrorCode(code, message)
{
}

GraphicsInitError::GraphicsInitError(int  code, std::string& message) noexcept
	: ErrorCode(code, message)
{

}

#pragma endregion

#pragma region CaptureScreenError

CaptureScreenError::CaptureScreenError() noexcept
	: ErrorCode()
{

}

CaptureScreenError::CaptureScreenError(int code, const char* message) noexcept
	: ErrorCode(code, message)
{
}

CaptureScreenError::CaptureScreenError(int  code, std::string& message) noexcept
	: ErrorCode(code, message)
{

}

#pragma endregion