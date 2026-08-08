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

    // Construction & Destruction

    Connection();

    ~Connection();


    // Non-copyable

    Connection(const Connection&) = delete;

    Connection& operator=(const Connection&) = delete;


    // Movable

    Connection(Connection&& other) noexcept;

    Connection& operator=(Connection&& other) noexcept;



    // Connection Management

    common::Error open();


    common::Error connect(
        const std::string& address,
        std::uint16_t port);



    void disconnect() noexcept;



    // State Inspection

    bool isConnected() const noexcept;



    // Socket Access

    Socket& socket() noexcept;

    const Socket& socket() const noexcept;



private:

    Socket socket_;

    bool connected_;

};


} // namespace packetforge::network