#pragma once

#include <cstdint>
#include <string>

#include "common/error.hpp"

namespace packetforge::network
{

class Socket
{
public:

    // Construction & Destruction

    Socket();
    ~Socket();


    // Non-copyable

    Socket(const Socket&) = delete;
    Socket& operator=(const Socket&) = delete;


    // Movable

    Socket(Socket&& other) noexcept;

    Socket& operator=(Socket&& other) noexcept;


    // Resource Management

    common::Error create();

    void close() noexcept;


    // Server Operations

    common::Error bind(
        const std::string& address,
        std::uint16_t port
    );

    common::Error listen(
        int backlog
    );

    common::Error accept(
        Socket& client
    );


    // State Inspection

    bool isOpen() const noexcept;

    int nativeHandle() const noexcept;

    private:

    int socket_;

};

} // namespace packetforge::network