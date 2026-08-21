#include "client/client.hpp"

#include <utility>


namespace packetforge::client
{


Client::Client()
:
transport_{},
packetIO_(transport_)
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
),
packetIO_(
    transport_
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

    /*
     * Packet serialization and transport are now handled
     * by PacketIO.
     */

    return packetIO_.send(packet);
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
     * Packet framing, payload extraction, and deserialization
     * are now handled by PacketIO.
     */

    return packetIO_.receive(packet);
}


} // namespace packetforge::client