#include "common/error.hpp"

#include <utility>

namespace packetforge::common
{


Error::Error(
    ErrorCode code,
    std::string message)
    :
    code_(code),
    message_(std::move(message))
{
}


ErrorCode Error::code() const noexcept
{
    return code_;
}


const std::string&
Error::message() const noexcept
{
    return message_;
}


bool Error::ok() const noexcept
{
    return code_ == ErrorCode::Success;
}


}