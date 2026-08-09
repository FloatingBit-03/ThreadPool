#include <gtest/gtest.h>
#include <cstring>

#include "protocol/encoder.hpp"
#include "common/endian.hpp"

using namespace packetforge::protocol;

class EncoderTest : public ::testing::Test
{
protected:

    Encoder encoder;
};

TEST_F(EncoderTest, EncodesEmptyPacket)
{
    Packet packet;

    auto bytes = encoder.encode(packet);

    EXPECT_EQ(bytes.size(), Packet::HEADER_SIZE);
}

TEST_F(EncoderTest, EncodesPayload)
{
    Packet packet;

    packet.setPayload(
    {
        1,
        2,
        3,
        4
    });

    auto bytes = encoder.encode(packet);

    EXPECT_EQ(
        bytes.size(),
        Packet::HEADER_SIZE + 4);
}

TEST_F(EncoderTest, HeaderContainsMagicNumber)
{
    Packet packet;

    auto bytes = encoder.encode(packet);

    std::uint32_t magic = 0;

    std::memcpy(
        &magic,
        bytes.data(),
        sizeof(magic));

    magic = packetforge::common::Endian::networkToHost(magic);
        
    EXPECT_EQ(
        magic,
        Packet::MagicNumber);
}

TEST_F(EncoderTest, HeaderContainsVersion)
{
    Packet packet;

    packet.setVersion(2);

    auto bytes = encoder.encode(packet);

    EXPECT_EQ(
        bytes[4],
        2);
}

TEST_F(EncoderTest, HeaderContainsFlags)
{
    Packet packet;

    packet.setFlags(0xAA);

    auto bytes = encoder.encode(packet);

    EXPECT_EQ(
        bytes[5],
        0xAA);
}

TEST_F(EncoderTest, PayloadBytesAppearAtEnd)
{
    Packet packet;

    packet.setPayload(
    {
        10,
        20,
        30
    });

    auto bytes = encoder.encode(packet);

    EXPECT_EQ(
        bytes[Packet::HEADER_SIZE],
        10);

    EXPECT_EQ(
        bytes[Packet::HEADER_SIZE + 1],
        20);

    EXPECT_EQ(
        bytes[Packet::HEADER_SIZE + 2],
        30);
}