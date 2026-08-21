#pragma once

#include <cstdint>

namespace packetforge::protocol
{

enum class Opcode : std::uint16_t
{
    HelloRequest  = 1,
    HelloResponse = 2
};

[[nodiscard]]
constexpr bool isValidOpcode(
    std::uint16_t value
) noexcept
{
    switch (static_cast<Opcode>(value))
    {
        case Opcode::HelloRequest:
        case Opcode::HelloResponse:
            return true;

        default:
            return false;
    }
}

} // namespace packetforge::protocol