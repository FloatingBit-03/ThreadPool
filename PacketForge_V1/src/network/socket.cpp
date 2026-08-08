#include "network/socket.hpp"

#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>


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



Socket::Socket(Socket&& other) noexcept
    :
    socket_(other.socket_)
{
    other.socket_ = -1;
}



Socket&
Socket::operator=(Socket&& other) noexcept
{
    if(this != &other)
    {
        close();

        socket_ = other.socket_;

        other.socket_ = -1;
    }


    return *this;
}



common::Error Socket::create()
{
    socket_ = ::socket(
        AF_INET,
        SOCK_STREAM,
        0
    );


    if(socket_ == -1)
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



void Socket::close() noexcept
{
    if(socket_ != -1)
    {
        ::close(socket_);

        socket_ = -1;
    }
}



bool Socket::isOpen() const noexcept
{
    return socket_ != -1;
}



int Socket::nativeHandle() const noexcept
{
    return socket_;
}


} // namespace packetforge::network