#pragma once

#include <cstdint>

namespace packetforge::protocol
{

// ==========================================================
// Packet Opcodes
// ==========================================================

enum class Opcode : std::uint16_t
{
    HelloRequest  = 1,
    HelloResponse = 2,

    ErrorResponse = 100
};


// ==========================================================
// Protocol Error Codes
// ==========================================================

enum class ProtocolError : std::uint16_t
{
    InvalidPacket      = 1,
    UnsupportedOpcode  = 2,
    UnsupportedVersion = 3,
    InvalidPayload     = 4
};


// ==========================================================
// Opcode Validation
// ==========================================================

[[nodiscard]]
constexpr bool isValidOpcode(
    std::uint16_t value
) noexcept
{
    switch (static_cast<Opcode>(value))
    {
        case Opcode::HelloRequest:
        case Opcode::HelloResponse:
        case Opcode::ErrorResponse:
            return true;

        default:
            return false;
    }
}


// ==========================================================
// Protocol Error Validation
// ==========================================================

[[nodiscard]]
constexpr bool isValidProtocolError(
    std::uint16_t value
) noexcept
{
    switch (static_cast<ProtocolError>(value))
    {
        case ProtocolError::InvalidPacket:
        case ProtocolError::UnsupportedOpcode:
        case ProtocolError::UnsupportedVersion:
        case ProtocolError::InvalidPayload:
            return true;

        default:
            return false;
    }
}

} // namespace packetforge::protocol