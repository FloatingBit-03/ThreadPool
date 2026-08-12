#include "protocol/decoder.hpp"

#include "common/endian.hpp"

#include <cstring>
#include <stdexcept>
#include <cstdint>
#include <vector>

namespace packetforge::protocol
{

namespace
{

std::uint16_t readNetworkUint16(
    const std::vector<std::uint8_t>& data,
    std::size_t offset)
{
    if (
        offset + sizeof(std::uint16_t) >
        data.size())
    {
        throw std::runtime_error(
            "Insufficient data for uint16"
        );
    }

    std::uint16_t value{};

    std::memcpy(
        &value,
        data.data() + offset,
        sizeof(value)
    );

    return common::Endian::networkToHost(value);
}

std::uint32_t readNetworkUint32(
    const std::vector<std::uint8_t>& data,
    std::size_t offset)
{
    if (
        offset + sizeof(std::uint32_t) >
        data.size())
    {
        throw std::runtime_error(
            "Insufficient data for uint32"
        );
    }

    std::uint32_t value{};

    std::memcpy(
        &value,
        data.data() + offset,
        sizeof(value)
    );

    return common::Endian::networkToHost(value);
}

} // anonymous namespace

Packet
Decoder::decode(
    const std::vector<std::uint8_t>& data)
{
    /*
     * Packet header layout:
     *
     * Offset  Size   Field
     * -------------------------
     * 0       4      Magic Number
     * 4       1      Version
     * 5       1      Flags
     * 6       2      Opcode
     * 8       4      Sequence ID
     * 12      4      Payload Length
     *
     * Total header = 16 bytes
     */

    if (data.size() < Packet::HEADER_SIZE)
    {
        throw std::runtime_error(
            "Invalid packet size"
        );
    }

    /*
     * Decode magic number.
     */

    const std::uint32_t magic =
        readNetworkUint32(data, 0);

    if (magic != Packet::MagicNumber)
    {
        throw std::runtime_error(
            "Invalid magic number"
        );
    }

    /*
     * Decode and validate version.
     */

    const std::uint8_t version = data[4];

    if (version != Packet::VERSION)
    {
        throw std::runtime_error(
            "Unsupported protocol version"
        );
    }

    /*
     * Create Packet after validating
     * the fixed protocol identifiers.
     */

    Packet packet;

    packet.setVersion(version);

    /*
     * Decode flags.
     */

    packet.setFlags(data[5]);

    /*
     * Decode opcode.
     */

    packet.setOpcode(
        readNetworkUint16(data, 6)
    );

    /*
     * Decode sequence ID.
     */

    packet.setSequenceId(
        readNetworkUint32(data, 8)
    );

    /*
     * Decode payload length.
     */

    const std::uint32_t payloadLength =
        readNetworkUint32(data, 12);

    /*
     * Validate actual packet size.
     */

    if (
        data.size() !=
        Packet::HEADER_SIZE + payloadLength)
    {
        throw std::runtime_error(
            "Payload size mismatch"
        );
    }

    /*
     * Extract payload.
     */

    std::vector<std::uint8_t> payload(
        data.begin() + Packet::HEADER_SIZE,
        data.end()
    );

    packet.setPayload(payload);

    /*
     * Final validation.
     */

    if (!packet.isValid())
    {
        throw std::runtime_error(
            "Decoded packet is invalid"
        );
    }

    return packet;
}

} // namespace packetforge::protocol