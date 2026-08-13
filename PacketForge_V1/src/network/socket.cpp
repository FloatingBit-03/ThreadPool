#include "network/socket.hpp"

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

namespace packetforge::network
{

Socket::Socket()
    :
    socket_(-1)
{
}


Socket::~Socket()
{
    close();
}


// ----------------------------------------------------------
// Move Constructor
// ----------------------------------------------------------

Socket::Socket(
    Socket&& other
) noexcept
    :
    socket_(other.socket_)
{
    other.socket_ = -1;
}


// ----------------------------------------------------------
// Move Assignment
// ----------------------------------------------------------

Socket&
Socket::operator=(
    Socket&& other
) noexcept
{
    if (this != &other)
    {
        close();

        socket_ = other.socket_;

        other.socket_ = -1;
    }

    return *this;
}


// ----------------------------------------------------------
// Create
// ----------------------------------------------------------

common::Error
Socket::create()
{
    if (isOpen())
    {
        return common::Error(
            common::ErrorCode::Success,
            ""
        );
    }

    socket_ =
        ::socket(
            AF_INET,
            SOCK_STREAM,
            0
        );

    if (socket_ == -1)
    {
        return common::Error(
            common::ErrorCode::SocketError,
            std::strerror(errno)
        );
    }

    return common::Error(
        common::ErrorCode::Success,
        ""
    );
}


// ----------------------------------------------------------
// Bind
// ----------------------------------------------------------

common::Error
Socket::bind(
    const std::string& address,
    std::uint16_t port
)
{
    if (!isOpen())
    {
        auto error = create();

        if (!error.ok())
        {
            return error;
        }
    }


    int reuse = 1;

    if (::setsockopt(
            socket_,
            SOL_SOCKET,
            SO_REUSEADDR,
            &reuse,
            sizeof(reuse)) < 0)
    {
        return common::Error(
            common::ErrorCode::SocketError,
            std::strerror(errno)
        );
    }


    sockaddr_in endpoint{};

    endpoint.sin_family =
        AF_INET;

    endpoint.sin_port =
        htons(port);


    if (::inet_pton(
            AF_INET,
            address.c_str(),
            &endpoint.sin_addr) != 1)
    {
        return common::Error(
            common::ErrorCode::InvalidArgument,
            "Invalid IPv4 address"
        );
    }


    if (::bind(
            socket_,
            reinterpret_cast<sockaddr*>(&endpoint),
            sizeof(endpoint)) < 0)
    {
        return common::Error(
            common::ErrorCode::SocketError,
            std::strerror(errno)
        );
    }


    return common::Error(
        common::ErrorCode::Success,
        ""
    );
}


// ----------------------------------------------------------
// Listen
// ----------------------------------------------------------

common::Error
Socket::listen(
    int backlog
)
{
    if (!isOpen())
    {
        return common::Error(
            common::ErrorCode::SocketError,
            "Socket is not open"
        );
    }


    if (backlog <= 0)
    {
        return common::Error(
            common::ErrorCode::InvalidArgument,
            "Invalid listen backlog"
        );
    }


    if (::listen(
            socket_,
            backlog) < 0)
    {
        return common::Error(
            common::ErrorCode::SocketError,
            std::strerror(errno)
        );
    }


    return common::Error(
        common::ErrorCode::Success,
        ""
    );
}


// ----------------------------------------------------------
// Accept
// ----------------------------------------------------------

common::Error
Socket::accept(
    Socket& client
)
{
    if (!isOpen())
    {
        return common::Error(
            common::ErrorCode::SocketError,
            "Socket is not open"
        );
    }


    int clientSocket =
        ::accept(
            socket_,
            nullptr,
            nullptr
        );


    if (clientSocket < 0)
    {
        return common::Error(
            common::ErrorCode::SocketError,
            std::strerror(errno)
        );
    }


    client.close();

    client.socket_ =
        clientSocket;


    return common::Error(
        common::ErrorCode::Success,
        ""
    );
}


// ----------------------------------------------------------
// Close
// ----------------------------------------------------------

void
Socket::close() noexcept
{
    if (socket_ != -1)
    {
        ::close(socket_);

        socket_ = -1;
    }
}


// ----------------------------------------------------------
// State Inspection
// ----------------------------------------------------------

bool
Socket::isOpen() const noexcept
{
    return socket_ != -1;
}


// ----------------------------------------------------------
// Native Handle
// ----------------------------------------------------------

int
Socket::nativeHandle() const noexcept
{
    return socket_;
}

} // namespace packetforge::network