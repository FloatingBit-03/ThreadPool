#pragma once

#include <cstdint>
#include <string>

#include "common/error.hpp"
#include "network/socket.hpp"


namespace packetforge::network
{

class Connection
{
public:

    Connection();
    ~Connection();

    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;

    Connection(Connection&& other) noexcept;
    Connection& operator=(Connection&& other) noexcept;

    // Client-side
    common::Error open();

    common::Error connect(
        const std::string& address,
        std::uint16_t port);

    // Server-side
    common::Error accept(
        Socket& listeningSocket);

    void disconnect() noexcept;

    bool isConnected() const noexcept;

    Socket& socket() noexcept;
    const Socket& socket() const noexcept;

private:

    Socket socket_;
    bool connected_;
};


} // namespace packetforge::network