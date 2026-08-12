#include <gtest/gtest.h>
#include <cstring>
#include <stdexcept>

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

    auto bytes = encoder.encode(packet);

    EXPECT_EQ(
        bytes[4],
        Packet::VERSION
    );
}

TEST_F(EncoderTest, RejectsInvalidVersion)
{
    Packet packet;

    packet.setVersion(2);

    EXPECT_FALSE(
        packet.isValid()
    );

    EXPECT_THROW(
        encoder.encode(packet),
        std::invalid_argument
    );
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

TEST_F(EncoderTest, HeaderContainsOpcode)
{
    Packet packet;

    packet.setOpcode(0x1234);

    auto bytes = encoder.encode(packet);

    std::uint16_t opcode = 0;

    std::memcpy(
        &opcode,
        bytes.data() + 6,
        sizeof(opcode)
    );

    opcode =
        packetforge::common::Endian::networkToHost(
            opcode
        );

    EXPECT_EQ(
        opcode,
        0x1234
    );
}


TEST_F(EncoderTest, HeaderContainsSequenceId)
{
    Packet packet;

    packet.setSequenceId(0x12345678);

    auto bytes = encoder.encode(packet);

    std::uint32_t sequenceId = 0;

    std::memcpy(
        &sequenceId,
        bytes.data() + 8,
        sizeof(sequenceId)
    );

    sequenceId =
        packetforge::common::Endian::networkToHost(
            sequenceId
        );

    EXPECT_EQ(
        sequenceId,
        0x12345678u
    );
}


TEST_F(EncoderTest, HeaderContainsPayloadLength)
{
    Packet packet;

    packet.setPayload(
    {
        10,
        20,
        30,
        40,
        50
    });

    auto bytes = encoder.encode(packet);

    std::uint32_t payloadLength = 0;

    std::memcpy(
        &payloadLength,
        bytes.data() + 12,
        sizeof(payloadLength)
    );

    payloadLength =
        packetforge::common::Endian::networkToHost(
            payloadLength
        );

    EXPECT_EQ(
        payloadLength,
        5u
    );
}


TEST_F(EncoderTest, EncodesCompletePacket)
{
    Packet packet;

    packet.setFlags(0xAA);
    packet.setOpcode(0x1234);
    packet.setSequenceId(0x12345678);

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
        Packet::HEADER_SIZE + 4
    );
}