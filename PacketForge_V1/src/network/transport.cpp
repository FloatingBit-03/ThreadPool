#include "network/transport.hpp"

#include <sys/socket.h>

namespace packetforge::network
{

Transport::Transport()
:
connection_{}
{
}


Transport::Transport(
    Connection connection
)
:
connection_(
    std::move(connection)
)
{
}


Transport::~Transport() = default;


// ----------------------------------------------------------
// Move Constructor
// ----------------------------------------------------------

Transport::Transport(
    Transport&& other
) noexcept
:
connection_(
    std::move(other.connection_)
)
{
}


// ----------------------------------------------------------
// Move Assignment
// ----------------------------------------------------------

Transport&
Transport::operator=(
    Transport&& other
) noexcept
{
    if (this != &other)
    {
        connection_ =
            std::move(other.connection_);
    }

    return *this;
}


// ----------------------------------------------------------
// Send
// ----------------------------------------------------------

common::Error
Transport::send(
    const std::vector<std::uint8_t>& data
)
{
    if (!connection_.isConnected())
    {
        return common::Error(
            common::ErrorCode::ConnectionFailed,
            "Connection not established"
        );
    }


    std::size_t totalSent = 0;


    while (totalSent < data.size())
    {
        auto result =
            ::send(
                connection_.socket().nativeHandle(),
                data.data() + totalSent,
                data.size() - totalSent,
                0
            );


        if (result < 0)
        {
            return common::Error(
                common::ErrorCode::SocketError,
                "Send failed"
            );
        }


        if (result == 0)
        {
            return common::Error(
                common::ErrorCode::SocketError,
                "Send failed: connection closed"
            );
        }


        totalSent +=
            static_cast<std::size_t>(result);
    }


    return common::Error(
        common::ErrorCode::Success,
        ""
    );
}


// ----------------------------------------------------------
// Receive
// ----------------------------------------------------------

common::Error
Transport::receive(
    std::vector<std::uint8_t>& data,
    std::size_t size
)
{
    if (!connection_.isConnected())
    {
        return common::Error(
            common::ErrorCode::ConnectionFailed,
            "Connection not established"
        );
    }


    data.resize(size);


    std::size_t totalReceived = 0;


    while (totalReceived < size)
    {
        auto result =
            ::recv(
                connection_.socket().nativeHandle(),
                data.data() + totalReceived,
                size - totalReceived,
                0
            );


        if (result < 0)
        {
            data.clear();

            return common::Error(
                common::ErrorCode::SocketError,
                "Receive failed"
            );
        }


        if (result == 0)
        {
            data.clear();

            return common::Error(
                common::ErrorCode::ConnectionClosed,
                "Connection closed by peer"
);
        }


        totalReceived +=
            static_cast<std::size_t>(result);
    }


    return common::Error(
        common::ErrorCode::Success,
        ""
    );
}


// ----------------------------------------------------------
// State
// ----------------------------------------------------------

bool
Transport::isConnected() const noexcept
{
    return connection_.isConnected();
}


// ----------------------------------------------------------
// Connection Access
// ----------------------------------------------------------

Connection&
Transport::connection() noexcept
{
    return connection_;
}

} // namespace packetforge::network