#include "protocol/decoder.hpp"

#include "common/endian.hpp"

#include <cstring>

namespace packetforge::protocol
{

// ==========================================================
// Decode Packet
// ==========================================================

Packet
Decoder::decode(
    const std::vector<std::uint8_t>& data
)
{
    // ------------------------------------------------------
    // Header size validation
    // ------------------------------------------------------

    if (data.size() < Packet::HEADER_SIZE)
    {
        throw ProtocolDecodeError(
            ProtocolError::InvalidPacket,
            "Invalid packet size"
        );
    }


    // ------------------------------------------------------
    // Magic number
    // ------------------------------------------------------

    const auto magic =
        readUint32(
            data,
            0
        );

    if (magic != Packet::MagicNumber)
    {
        throw ProtocolDecodeError(
            ProtocolError::InvalidPacket,
            "Invalid magic number"
        );
    }


    // ------------------------------------------------------
    // Version
    // ------------------------------------------------------

    const auto version =
        data[4];

    if (version != Packet::VERSION)
    {
        throw ProtocolDecodeError(
            ProtocolError::UnsupportedVersion,
            "Unsupported protocol version"
        );
    }


    // ------------------------------------------------------
    // Flags
    // ------------------------------------------------------

    const auto flags =
        data[5];


    // ------------------------------------------------------
    // Opcode
    // ------------------------------------------------------

    const auto opcode =
        readUint16(
            data,
            6
        );


    // ------------------------------------------------------
    // Sequence ID
    // ------------------------------------------------------

    const auto sequenceId =
        readUint32(
            data,
            8
        );


    // ------------------------------------------------------
    // Payload length
    // ------------------------------------------------------

    const auto payloadLength =
        readUint32(
            data,
            12
        );


    // ------------------------------------------------------
    // Validate complete packet size
    // ------------------------------------------------------

    const std::size_t expectedSize =
        Packet::HEADER_SIZE +
        static_cast<std::size_t>(payloadLength);

    if (data.size() != expectedSize)
    {
        throw ProtocolDecodeError(
            ProtocolError::InvalidPayload,
            "Payload size mismatch"
        );
    }


    // ------------------------------------------------------
    // Construct Packet
    // ------------------------------------------------------

    Packet packet;

    packet.setVersion(
        version
    );

    packet.setFlags(
        flags
    );

    packet.setOpcode(
        opcode
    );

    packet.setSequenceId(
        sequenceId
    );


    // ------------------------------------------------------
    // Extract payload
    // ------------------------------------------------------

    if (payloadLength > 0)
    {
        std::vector<std::uint8_t> payload(
            data.begin() + Packet::HEADER_SIZE,
            data.end()
        );

        packet.setPayload(
            payload
        );
    }


    return packet;
}


// ==========================================================
// Read uint32
// ==========================================================

std::uint32_t
Decoder::readUint32(
    const std::vector<std::uint8_t>& data,
    std::size_t offset
)
{
    std::uint32_t value{};

    std::memcpy(
        &value,
        data.data() + offset,
        sizeof(value)
    );

    return common::Endian::networkToHost(
        value
    );
}


// ==========================================================
// Read uint16
// ==========================================================

std::uint16_t
Decoder::readUint16(
    const std::vector<std::uint8_t>& data,
    std::size_t offset
)
{
    std::uint16_t value{};

    std::memcpy(
        &value,
        data.data() + offset,
        sizeof(value)
    );

    return common::Endian::networkToHost(
        value
    );
}

} // namespace packetforge::protocol