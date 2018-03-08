#include "errors.h"

using namespace hotk::errors;

ErrorCode::ErrorCode() noexcept
    : _code(0)
    , runtime_error("")
{
}

ErrorCode::ErrorCode(int code, const char* message) noexcept
    : _code(code)
    , runtime_error(message)
{
}

ErrorCode::ErrorCode(int code, std::string& message) noexcept
    : _code(code)
    , runtime_error(message)
{
}

int ErrorCode::code() const noexcept
{
    return _code;
}

ErrorCode::operator bool() const noexcept
{
    return _code != 0 || strlen(this->what()) > 0;
}
