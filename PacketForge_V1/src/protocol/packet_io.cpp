#include "protocol/packet_io.hpp"

#include "common/endian.hpp"
#include "protocol/decoder.hpp"
#include "protocol/encoder.hpp"

#include <cstring>
#include <exception>
#include <stdexcept>
#include <vector>

namespace packetforge::protocol
{

PacketIO::PacketIO(
    network::Transport& transport
)
    :
    transport_(transport)
{
}


// ==========================================================
// Send
// ==========================================================

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

        return transport_.send(
            data
        );
    }
    catch (const std::invalid_argument& error)
    {
        return common::Error(
            common::ErrorCode::SerializationError,
            error.what()
        );
    }
    catch (const std::exception& error)
    {
        return common::Error(
            common::ErrorCode::UnknownError,
            error.what()
        );
    }
}


// ==========================================================
// Receive
// ==========================================================

common::Error
PacketIO::receive(
    Packet& packet,
    ProtocolError* protocolError
)
{
    // ------------------------------------------------------
    // Clear previous protocol error
    // ------------------------------------------------------

    if (protocolError != nullptr)
    {
        *protocolError =
            ProtocolError::InvalidPacket;
    }


    // ------------------------------------------------------
    // Receive fixed header
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
    // Extract payload length
    // ------------------------------------------------------

    std::uint32_t networkPayloadLength{};

    std::memcpy(
        &networkPayloadLength,
        header.data() + 12,
        sizeof(networkPayloadLength)
    );

    const auto payloadLength =
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
    // Reconstruct complete packet
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
    // Decode
    // ------------------------------------------------------

    try
    {
        packet =
            Decoder::decode(
                data
            );
    }
    catch (const ProtocolDecodeError& error)
    {
        if (protocolError != nullptr)
        {
            *protocolError =
                error.error();
        }

        return common::Error(
            common::ErrorCode::ProtocolError,
            error.what()
        );
    }
    catch (const std::exception& error)
    {
        return common::Error(
            common::ErrorCode::DeserializationError,
            error.what()
        );
    }


    return common::Error(
        common::ErrorCode::Success,
        ""
    );
}

} // namespace packetforge::protocol