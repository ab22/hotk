#include "errors.h"

using namespace hotk::winutils::errors;

Win32Error::Win32Error() noexcept
    : ErrorCode()
{
}

Win32Error::Win32Error(int code, const char* message) noexcept
    : ErrorCode(code, message)
{
}

Win32Error::Win32Error(int code, std::string& message) noexcept
    : ErrorCode(code, message)
{
}
