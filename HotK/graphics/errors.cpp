#include "errors.h"

using namespace hotk::graphics;

#pragma region GDIStartupError

GDIStartupError::GDIStartupError() noexcept
	: ErrorCode()
{

}

GDIStartupError::GDIStartupError(int code, const char* message) noexcept
	: ErrorCode(code, message)
{
}

GDIStartupError::GDIStartupError(int  code, std::string& message) noexcept
	: ErrorCode(code, message)
{

}

#pragma endregion

#pragma region EncoderClsIDError

EncoderClsIDError::EncoderClsIDError() noexcept
	: ErrorCode()
{

}

EncoderClsIDError::EncoderClsIDError(int code, const char* message) noexcept
	: ErrorCode(code, message)
{
}

EncoderClsIDError::EncoderClsIDError(int  code, std::string& message) noexcept
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