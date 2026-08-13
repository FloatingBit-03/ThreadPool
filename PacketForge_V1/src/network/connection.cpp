#include "network/connection.hpp"


#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>


namespace packetforge::network
{


Connection::Connection()
    :
    socket_{},
    connected_{false}
{
}



Connection::~Connection()
{
    disconnect();
}



Connection::Connection(Connection&& other) noexcept
    :
    socket_(std::move(other.socket_)),
    connected_(other.connected_)
{
    other.connected_ = false;
}



Connection&
Connection::operator=(Connection&& other) noexcept
{
    if (this != &other)
    {

        disconnect();


        socket_ =
            std::move(other.socket_);


        connected_ =
            other.connected_;


        other.connected_ = false;
    }


    return *this;
}



common::Error
Connection::open()
{
    return socket_.create();
}



common::Error
Connection::connect(
    const std::string& address,
    std::uint16_t port)
{

    if (!socket_.isOpen())
    {

        auto error =
            socket_.create();


        if (!error.ok())
        {
            return error;
        }
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



    if (::connect(
            socket_.nativeHandle(),
            reinterpret_cast<sockaddr*>(&endpoint),
            sizeof(endpoint)) < 0)
    {

        return common::Error(
            common::ErrorCode::ConnectionFailed,
            std::strerror(errno)
        );
    }



    connected_ = true;



    return common::Error(
        common::ErrorCode::Success,
        ""
    );
}


common::Error
Connection::accept(
    Socket& listeningSocket)
{
    if (!listeningSocket.isOpen())
    {
        return common::Error(
            common::ErrorCode::SocketError,
            "Listening socket is not open"
        );
    }

    if (connected_)
    {
        return common::Error(
            common::ErrorCode::ConnectionFailed,
            "Connection is already established"
        );
    }

    auto error =
        listeningSocket.accept(socket_);

    if (!error.ok())
    {
        return error;
    }

    connected_ = true;

    return common::Error(
        common::ErrorCode::Success,
        ""
    );
}

void
Connection::disconnect() noexcept
{

    if (socket_.isOpen())
    {
        socket_.close();
    }


    connected_ = false;
}



bool
Connection::isConnected() const noexcept
{
    return connected_;
}



Socket&
Connection::socket() noexcept
{
    return socket_;
}



const Socket&
Connection::socket() const noexcept
{
    return socket_;
}


} // namespace packetforge::network