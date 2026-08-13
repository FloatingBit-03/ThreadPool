#include "server/server.hpp"

#include <cerrno>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <utility>

namespace packetforge::server
{

Server::Server()
    :
    socket_{},
    running_{false},
    port_{0}
{
}

Server::~Server()
{
    stop();
}

Server::Server(
    Server&& other
) noexcept
    :
    socket_(
        std::move(other.socket_)
    ),
    running_(
        other.running_
    ),
    port_(
        other.port_
    )
{
    other.running_ = false;
    other.port_ = 0;
}

Server&
Server::operator=(
    Server&& other
) noexcept
{
    if (this != &other)
    {
        stop();

        socket_ =
            std::move(other.socket_);

        running_ =
            other.running_;

        port_ =
            other.port_;

        other.running_ = false;
        other.port_ = 0;
    }

    return *this;
}

common::Error
Server::start(
    const std::string& address,
    std::uint16_t port
)
{
    if (running_)
    {
        return common::Error(
            common::ErrorCode::SocketError,
            "Server is already running"
        );
    }

    auto error =
        socket_.create();

    if (!error.ok())
    {
        port_ = 0;
        return error;
    }

    error =
        socket_.bind(
            address,
            port
        );

    if (!error.ok())
    {
        socket_.close();
        port_ = 0;

        return error;
    }

    error =
        socket_.listen(
            SOMAXCONN
        );

    if (!error.ok())
    {
        socket_.close();
        port_ = 0;

        return error;
    }

    sockaddr_in addressInfo{};

    socklen_t addressLength =
        sizeof(addressInfo);

    if (::getsockname(
            socket_.nativeHandle(),
            reinterpret_cast<sockaddr*>(&addressInfo),
            &addressLength
        ) < 0)
    {
        const auto message =
            std::strerror(errno);

        socket_.close();
        port_ = 0;

        return common::Error(
            common::ErrorCode::SocketError,
            message
        );
    }

    port_ =
        ntohs(
            addressInfo.sin_port
        );

    running_ = true;

    return common::Error(
        common::ErrorCode::Success,
        ""
    );
}

void
Server::stop() noexcept
{
    if (socket_.isOpen())
    {
        socket_.close();
    }

    running_ = false;
    port_ = 0;
}

bool
Server::isRunning() const noexcept
{
    return running_;
}

std::uint16_t
Server::port() const noexcept
{
    return port_;
}

common::Error
Server::accept(
    network::Connection& connection
)
{
    if (!running_)
    {
        return common::Error(
            common::ErrorCode::ConnectionFailed,
            "Server is not running"
        );
    }

    return connection.accept(
        socket_
    );
}

} // namespace packetforge::server