#pragma once

#include "common/error.hpp"
#include "network/transport.hpp"
#include "protocol/packet.hpp"

#include <cstdint>
#include <string>

namespace packetforge::client
{

class Client
{
public:

    // Construction & Destruction

    Client();
    ~Client();

    // Non-copyable

    Client(const Client&) = delete;
    Client& operator=(const Client&) = delete;

    // Movable

    Client(Client&& other) noexcept;
    Client& operator=(Client&& other) noexcept;

    // Connection Management

    common::Error connect(
        const std::string& host,
        std::uint16_t port);

    void disconnect() noexcept;

    bool isConnected() const noexcept;

    // Packet Communication

    common::Error send(
        const protocol::Packet& packet);

    common::Error receive(
        protocol::Packet& packet);

private:

    network::Transport transport_;
};

} // namespace packetforge::client