#ifndef PACKETFORGE_PROTOCOL_ENCODER_HPP
#define PACKETFORGE_PROTOCOL_ENCODER_HPP

#include <vector>
#include <cstdint>

#include "protocol/packet.hpp"

namespace packetforge::protocol
{

class Encoder
{
public:
    
    Encoder() = default;

    [[nodiscard]]
    std::vector<std::uint8_t>
    encode(const Packet& packet) const;

};

}

#endif