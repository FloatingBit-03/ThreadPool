#include "client/client.hpp"

#include "common/endian.hpp"
#include "protocol/encoder.hpp"
#include "protocol/decoder.hpp"

#include <cstring>
#include <exception>
#include <utility>
#include <vector>


namespace packetforge::client
{


Client::Client()
:
transport_{}
{
}


Client::~Client() = default;


// ==========================================================
// Move Constructor
// ==========================================================

Client::Client(
    Client&& other
) noexcept
:
transport_(
    std::move(other.transport_)
)
{
}


// ==========================================================
// Move Assignment
// ==========================================================

Client&
Client::operator=(
    Client&& other
) noexcept
{
    if(this != &other)
    {
        transport_ =
            std::move(other.transport_);
    }

    return *this;
}


// ==========================================================
// Connection Management
// ==========================================================

common::Error
Client::connect(
    const std::string& host,
    std::uint16_t port
)
{
    return transport_.connection().connect(
        host,
        port
    );
}


void
Client::disconnect() noexcept
{
    transport_.connection().disconnect();
}


bool
Client::isConnected() const noexcept
{
    return transport_.isConnected();
}


// ==========================================================
// Packet Communication
// ==========================================================

common::Error
Client::send(
    const protocol::Packet& packet
)
{
    if(!isConnected())
    {
        return common::Error(
            common::ErrorCode::ConnectionFailed,
            "Connection not established"
        );
    }


    protocol::Encoder encoder;


    std::vector<std::uint8_t> data;


    try
    {
        data =
            encoder.encode(packet);
    }
    catch(const std::exception& exception)
    {
        return common::Error(
            common::ErrorCode::SerializationError,
            exception.what()
        );
    }


    return transport_.send(data);
}


// ==========================================================
// Receive Packet
// ==========================================================

common::Error
Client::receive(
    protocol::Packet& packet
)
{
    if(!isConnected())
    {
        return common::Error(
            common::ErrorCode::ConnectionFailed,
            "Connection not established"
        );
    }


    /*
     * Phase 3 defines a fixed-size 16-byte header.
     *
     * Receive the header first so that the payload
     * length can be determined.
     */

    std::vector<std::uint8_t> header;


    auto result =
        transport_.receive(
            header,
            protocol::Packet::HEADER_SIZE
        );


    if(!result.ok())
    {
        return result;
    }


    if(header.size() != protocol::Packet::HEADER_SIZE)
    {
        return common::Error(
            common::ErrorCode::DeserializationError,
            "Incomplete packet header"
        );
    }


    /*
     * Payload Length is stored at offset 12.
     *
     * Wire format:
     *
     * 0   - 3   Magic Number
     * 4         Version
     * 5         Flags
     * 6   - 7   Opcode
     * 8   - 11  Sequence ID
     * 12  - 15  Payload Length
     */

    std::uint32_t payloadLength = 0;


    std::memcpy(
        &payloadLength,
        header.data() + 12,
        sizeof(payloadLength)
    );


    payloadLength =
        common::Endian::networkToHost(
            payloadLength
        );


    /*
     * Receive the payload.
     */

    std::vector<std::uint8_t> payload;


    if(payloadLength > 0)
    {
        result =
            transport_.receive(
                payload,
                payloadLength
            );


        if(!result.ok())
        {
            return result;
        }


        if(payload.size() != payloadLength)
        {
            return common::Error(
                common::ErrorCode::DeserializationError,
                "Incomplete packet payload"
            );
        }
    }


    /*
     * Reconstruct the complete wire-format packet.
     */

    std::vector<std::uint8_t> data;


    data.reserve(
        protocol::Packet::HEADER_SIZE +
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


    /*
     * Let the Phase 3 Decoder perform protocol
     * validation and construct the Packet.
     */

    try
    {
        packet =
            protocol::Decoder::decode(data);
    }
    catch(const std::exception& exception)
    {
        return common::Error(
            common::ErrorCode::DeserializationError,
            exception.what()
        );
    }


    return common::Error(
        common::ErrorCode::Success,
        ""
    );
}


} // namespace packetforge::client