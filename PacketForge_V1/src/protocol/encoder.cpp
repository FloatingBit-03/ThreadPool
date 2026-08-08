#include "protocol/encoder.hpp"

#include <cstring>

namespace packetforge::protocol
{

namespace
{

template<typename T>
void appendValue(std::vector<std::uint8_t>& buffer, T value)
{
    const auto* bytes =
        reinterpret_cast<const std::uint8_t*>(&value);

    buffer.insert(buffer.end(), bytes, bytes + sizeof(T));
}

}

std::vector<std::uint8_t>
Encoder::encode(const Packet& packet) const
{
    std::vector<std::uint8_t> buffer;

    buffer.reserve(
        Packet::HEADER_SIZE +
        packet.payloadLength());

    appendValue(buffer, packet.magicNumber());

    appendValue(buffer, packet.version());

    appendValue(buffer, packet.flags());

    appendValue(buffer, packet.opcode());

    appendValue(buffer, packet.sequenceId());

    appendValue(buffer, packet.payloadLength());

    const auto& payload = packet.payload();

    buffer.insert(
        buffer.end(),
        payload.begin(),
        payload.end());

    return buffer;
}

}