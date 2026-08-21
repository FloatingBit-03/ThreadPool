#pragma once

#include "common/error.hpp"
#include "network/connection.hpp"
#include "network/socket.hpp"

#include <cstdint>
#include <string>

namespace packetforge::server
{

class Server
{
public:

    Server();
    ~Server();

    // Non-copyable

    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    // Movable

    Server(Server&& other) noexcept;
    Server& operator=(Server&& other) noexcept;

    // Server lifecycle

    common::Error start(
        const std::string& address,
        std::uint16_t port
    );

    void stop() noexcept;

    bool isRunning() const noexcept;

    std::uint16_t port() const noexcept;

    // Connection management

    common::Error accept(
        network::Connection& connection
    );

private:

    network::Socket socket_;
    bool running_;
    std::uint16_t port_;
};

} // namespace packetforge::server