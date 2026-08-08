#include "network/transport.hpp"


#include <sys/socket.h>


namespace packetforge::network
{


Transport::Transport()
    :
    connection_{}
{
}



Transport::Transport(
    Connection connection
)
    :
    connection_(
        std::move(connection)
    )
{
}



Transport::~Transport() = default;



Transport::Transport(
    Transport&& other
) noexcept
    :
    connection_(
        std::move(other.connection_)
    )
{
}



Transport&
Transport::operator=(
    Transport&& other
) noexcept
{

    if(this != &other)
    {
        connection_ =
            std::move(other.connection_);
    }


    return *this;
}



common::Error
Transport::send(
    const std::vector<std::uint8_t>& data
)
{

    if(!connection_.isConnected())
    {
        return common::Error(
            common::ErrorCode::ConnectionFailed,
            "Connection not established"
        );
    }



    auto result =
        ::send(
            connection_.socket().nativeHandle(),
            data.data(),
            data.size(),
            0
        );



    if(result < 0)
    {
        return common::Error(
            common::ErrorCode::SocketError,
            "Send failed"
        );
    }



    return common::Error(
        common::ErrorCode::Success,
        ""
    );
}



common::Error
Transport::receive(
    std::vector<std::uint8_t>& data,
    std::size_t size
)
{

    if(!connection_.isConnected())
    {
        return common::Error(
            common::ErrorCode::ConnectionFailed,
            "Connection not established"
        );
    }



    data.resize(size);



    auto result =
        ::recv(
            connection_.socket().nativeHandle(),
            data.data(),
            size,
            0
        );



    if(result <= 0)
    {
        return common::Error(
            common::ErrorCode::SocketError,
            "Receive failed"
        );
    }



    data.resize(result);



    return common::Error(
        common::ErrorCode::Success,
        ""
    );
}



bool
Transport::isConnected() const noexcept
{
    return connection_.isConnected();
}



Connection&
Transport::connection() noexcept
{
    return connection_;
}


}