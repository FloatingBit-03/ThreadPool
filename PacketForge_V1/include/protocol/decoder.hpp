#pragma once

#include <vector>
#include <cstdint>

#include "protocol/packet.hpp"


namespace packetforge::protocol
{

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

}