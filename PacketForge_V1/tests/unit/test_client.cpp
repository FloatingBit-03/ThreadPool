#include <gtest/gtest.h>

#include "client/client.hpp"

using packetforge::client::Client;
using packetforge::protocol::Packet;


TEST(
    ClientTest,
    ClientStartsDisconnected
)
{
    Client client;

    EXPECT_FALSE(
        client.isConnected()
    );
}


TEST(
    ClientTest,
    DisconnectWhenAlreadyDisconnectedIsSafe
)
{
    Client client;

    EXPECT_NO_THROW(
        client.disconnect()
    );

    EXPECT_FALSE(
        client.isConnected()
    );
}


TEST(
    ClientTest,
    SendFailsWhenDisconnected
)
{
    Client client;
    Packet packet;

    auto error =
        client.send(packet);

    EXPECT_FALSE(
        error.ok()
    );
}


TEST(
    ClientTest,
    MoveConstructorTransfersState
)
{
    Client client;

    Client moved(
        std::move(client)
    );

    EXPECT_FALSE(
        moved.isConnected()
    );
}


TEST(
    ClientTest,
    MoveAssignmentTransfersState
)
{
    Client client;
    Client moved;

    moved =
        std::move(client);

    EXPECT_FALSE(
        moved.isConnected()
    );
}