#include <gtest/gtest.h>

#include "protocol/packet.hpp"

using packetforge::protocol::Packet;

TEST(PacketTest, DefaultPacketIsValid)
{
    Packet packet;

    EXPECT_TRUE(
        packet.isValid()
    );

    EXPECT_EQ(
        packet.magicNumber(),
        Packet::MagicNumber
    );
}

TEST(PacketTest, DefaultVersionIsOne)
{
    Packet packet;

    EXPECT_EQ(
        packet.version(),
        1
    );
}

TEST(PacketTest, CanSetVersion)
{
    Packet packet;

    packet.setVersion(2);

    EXPECT_EQ(
        packet.version(),
        2
    );
}

TEST(PacketTest, CanSetFlags)
{
    Packet packet;

    packet.setFlags(0xAA);

    EXPECT_EQ(
        packet.flags(),
        0xAA
    );
}

TEST(PacketTest, CanSetOpcode)
{
    Packet packet;

    packet.setOpcode(100);

    EXPECT_EQ(
        packet.opcode(),
        100
    );
}

TEST(PacketTest, CanSetSequenceId)
{
    Packet packet;

    packet.setSequenceId(500);

    EXPECT_EQ(
        packet.sequenceId(),
        500u
    );
}

TEST(PacketTest, EmptyPayloadInitially)
{
    Packet packet;

    EXPECT_TRUE(
        packet.payload().empty()
    );

    EXPECT_EQ(
        packet.payloadLength(),
        0u
    );
}

TEST(PacketTest, StoresPayloadCorrectly)
{
    Packet packet;

    std::vector<std::uint8_t> payload =
    {
        10,
        20,
        30,
        40
    };

    packet.setPayload(payload);

    EXPECT_EQ(
        packet.payloadLength(),
        4u
    );

    EXPECT_EQ(
        packet.payload(),
        payload
    );
}

TEST(PacketTest, PayloadLengthMatchesSize)
{
    Packet packet;

    packet.setPayload(
    {
        1,
        2,
        3,
        4,
        5
    });

    EXPECT_EQ(
        packet.payloadLength(),
        5u
    );
}