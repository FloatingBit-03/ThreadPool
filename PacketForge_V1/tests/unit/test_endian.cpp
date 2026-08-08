#include <gtest/gtest.h>

#include "common/endian.hpp"

using packetforge::common::Endian;
using packetforge::common::Endianness;


TEST(EndianTest, DetectsHostEndianness)
{
    EXPECT_TRUE(
        Endian::hostEndianness() == Endianness::Little ||
        Endian::hostEndianness() == Endianness::Big);
}


TEST(EndianTest, ConvertsUint16)
{
    constexpr std::uint16_t value = 0x1234;

    auto network = Endian::hostToNetwork(value);
    auto host = Endian::networkToHost(network);

    EXPECT_EQ(host, value);
}


TEST(EndianTest, ConvertsUint32)
{
    constexpr std::uint32_t value = 0x12345678;

    auto network = Endian::hostToNetwork(value);
    auto host = Endian::networkToHost(network);

    EXPECT_EQ(host, value);
}


TEST(EndianTest, ConvertsUint64)
{
    constexpr std::uint64_t value = 0x1122334455667788ULL;

    auto network = Endian::hostToNetwork(value);
    auto host = Endian::networkToHost(network);

    EXPECT_EQ(host, value);
}