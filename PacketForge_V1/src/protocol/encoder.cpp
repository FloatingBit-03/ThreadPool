#include "protocol/encoder.hpp"

#include "common/endian.hpp"

namespace packetforge::protocol
{

namespace
{

template<typename T>
void appendValue(
    std::vector<std::uint8_t>& buffer,
    T value)
{
    const auto* bytes =
        reinterpret_cast<const std::uint8_t*>(&value);

    buffer.insert(
        buffer.end(),
        bytes,
        bytes + sizeof(T));
}

}

std::vector<std::uint8_t>
Encoder::encode(const Packet& packet) const
{
    std::vector<std::uint8_t> buffer;

    buffer.reserve(
        Packet::HEADER_SIZE +
        packet.payloadLength());

    appendValue(
        buffer,
        common::Endian::hostToNetwork(
            packet.magicNumber()));

    buffer.push_back(
        packet.version());

    buffer.push_back(
        packet.flags());

    appendValue(
        buffer,
        common::Endian::hostToNetwork(
            packet.opcode()));

    appendValue(
        buffer,
        common::Endian::hostToNetwork(
            packet.sequenceId()));

    appendValue(
        buffer,
        common::Endian::hostToNetwork(
            packet.payloadLength()));

    const auto& payload =
        packet.payload();

    buffer.insert(
        buffer.end(),
        payload.begin(),
        payload.end());

    return buffer;
}

}