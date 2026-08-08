#pragma once

#include <string>

namespace packetforge::common
{

enum class ErrorCode
{
    Success = 0,

    UnknownError,

    InvalidArgument,

    ConnectionFailed,

    SocketError,

    Timeout,

    InvalidPacket,

    SerializationError,

    DeserializationError
};


class Error
{
public:

    Error(
        ErrorCode code,
        std::string message);


    ErrorCode code() const noexcept;


    const std::string& message() const noexcept;


    bool ok() const noexcept;


private:

    ErrorCode code_;

    std::string message_;
};


}