#include "protocol/packet_io.hpp"

#include "common/endian.hpp"
#include "protocol/decoder.hpp"
#include "protocol/encoder.hpp"

#include <cstring>
#include <stdexcept>
#include <vector>
#include <exception>

namespace packetforge::protocol
{

PacketIO::PacketIO(
    network::Transport& transport
)
    :
    transport_(transport)
{
}

common::Error
PacketIO::send(
    const Packet& packet
)
{
    try
    {
        Encoder encoder;

        const auto data =
            encoder.encode(packet);

        return transport_.send(data);
    }
    catch (const std::exception& error)
    {
        return common::Error(
            common::ErrorCode::UnknownError,
            error.what()
        );
    }
}

common::Error
PacketIO::receive(
    Packet& packet
)
{
    // ------------------------------------------------------
    // Receive fixed-size packet header
    // ------------------------------------------------------

    std::vector<std::uint8_t> header;

    auto error =
        transport_.receive(
            header,
            Packet::HEADER_SIZE
        );

    if (!error.ok())
    {
        return error;
    }

    // ------------------------------------------------------
    // Extract payload length from header
    //
    // Header layout:
    //
    // 0-3    Magic Number
    // 4      Version
    // 5      Flags
    // 6-7    Opcode
    // 8-11   Sequence ID
    // 12-15  Payload Length
    // ------------------------------------------------------

    std::uint32_t networkPayloadLength{};

    std::memcpy(
        &networkPayloadLength,
        header.data() + 12,
        sizeof(networkPayloadLength)
    );

    const std::uint32_t payloadLength =
        common::Endian::networkToHost(
            networkPayloadLength
        );

    // ------------------------------------------------------
    // Receive payload
    // ------------------------------------------------------

    std::vector<std::uint8_t> payload;

    if (payloadLength > 0)
    {
        error =
            transport_.receive(
                payload,
                payloadLength
            );

        if (!error.ok())
        {
            return error;
        }
    }

    // ------------------------------------------------------
    // Reconstruct complete encoded packet
    // ------------------------------------------------------

    std::vector<std::uint8_t> data;

    data.reserve(
        Packet::HEADER_SIZE +
        payload.size()
    );

    data.insert(
        data.end(),
        header.begin(),
        header.end()
    );

    data.insert(
        data.end(),
        payload.begin(),
        payload.end()
    );

    // ------------------------------------------------------
    // Decode packet
    // ------------------------------------------------------

    try
    {
        packet =
            Decoder::decode(data);
    }
    catch (const std::exception& error)
    {
        return common::Error(
            common::ErrorCode::UnknownError,
            error.what()
        );
    }

    return common::Error(
        common::ErrorCode::Success,
        ""
    );
}

} // namespace packetforge::protocol