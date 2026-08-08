#pragma once

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
  
    // State Inspection
 
    bool isOpen() const noexcept;
    int nativeHandle() const noexcept;


private:

    int socket_;
};

} // namespace packetforge::network