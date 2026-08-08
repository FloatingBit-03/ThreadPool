#include <gtest/gtest.h>

#include "network/transport.hpp"

using packetforge::network::Transport;
using packetforge::common::ErrorCode;

TEST(TransportTest, StartsDisconnected)
{

    Transport transport;


    EXPECT_FALSE(
        transport.isConnected()
    );

}


TEST(TransportTest, SendFailsWithoutConnection)
{

    Transport transport;

    std::vector<std::uint8_t> data =
    {
        1,2,3
    };

    auto error =
        transport.send(data);

    EXPECT_EQ(
        error.code(),
        ErrorCode::ConnectionFailed
    );

}


TEST(TransportTest, ReceiveFailsWithoutConnection)
{

    Transport transport;

    std::vector<std::uint8_t> data;

    auto error =
        transport.receive(
            data,
            10
        );

    EXPECT_EQ(
        error.code(),
        ErrorCode::ConnectionFailed
    );

}


TEST(TransportTest, MoveConstructorWorks)
{
    Transport first;

    Transport second(
        std::move(first)
    );

    EXPECT_FALSE(
        second.isConnected()
    );

}


TEST(TransportTest, MoveAssignmentWorks)
{
    Transport first;

    Transport second;

    second =
        std::move(first);

    EXPECT_FALSE(
        second.isConnected()
    );

}