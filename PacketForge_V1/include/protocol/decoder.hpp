#pragma once

#include <cstdint>
#include <stdexcept>
#include <vector>

#include "protocol/opcode.hpp"
#include "protocol/packet.hpp"

namespace packetforge::protocol
{

class ProtocolDecodeError : public std::runtime_error
{
public:

    ProtocolDecodeError(
        ProtocolError error,
        const char* message
    )
        :
        std::runtime_error(message),
        error_(error)
    {
    }

    [[nodiscard]]
    ProtocolError error() const noexcept
    {
        return error_;
    }

private:

    ProtocolError error_;
};


class Decoder
{
public:

    static Packet decode(
        const std::vector<std::uint8_t>& data
    );


private:

    static std::uint32_t readUint32(
        const std::vector<std::uint8_t>& data,
        std::size_t offset
    );

    static std::uint16_t readUint16(
        const std::vector<std::uint8_t>& data,
        std::size_t offset
    );
};

} // namespace packetforge::protocol