#pragma once

#include <cstdint>
#include <string>

#include "common/error.hpp"
#include "network/connection.hpp"
#include "network/socket.hpp"

namespace packetforge::server
{

class Server
{
public:

    // Construction & Destruction

    Server();
    ~Server();


    // Non-copyable

    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;


    // Movable

    Server(Server&& other) noexcept;
    Server& operator=(Server&& other) noexcept;


    // Server Lifecycle

    common::Error start(
        const std::string& address,
        std::uint16_t port
    );

    void stop() noexcept;

    bool isRunning() const noexcept;

    std::uint16_t port() const noexcept;


    // Client Connection

    common::Error accept(
        network::Connection& connection
    );


private:

    network::Socket socket_;

    bool running_;

    std::uint16_t port_;
};

} // namespace packetforge::server