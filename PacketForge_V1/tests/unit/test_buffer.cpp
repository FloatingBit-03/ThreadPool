#include <gtest/gtest.h>

#include "common/buffer.hpp"


using packetforge::common::Buffer;



TEST(BufferTest, WritesAndReadsData)
{
    Buffer buffer;


    std::uint8_t input[]
    {
        10,
        20,
        30
    };


    buffer.write(
        input,
        3);


    EXPECT_EQ(
        buffer.size(),
        3);


    std::uint8_t output[3]{};


    bool result =
        buffer.read(
            output,
            3);


    EXPECT_TRUE(result);


    EXPECT_EQ(
        output[0],
        10);


    EXPECT_EQ(
        output[1],
        20);


    EXPECT_EQ(
        output[2],
        30);
}



TEST(BufferTest, CannotReadMoreThanAvailable)
{
    Buffer buffer;


    std::uint8_t input[]
    {
        1,
        2
    };


    buffer.write(
        input,
        2);


    std::uint8_t output[5]{};


    EXPECT_FALSE(
        buffer.read(
            output,
            5));
}



TEST(BufferTest, ClearRemovesData)
{
    Buffer buffer;


    std::uint8_t data[]
    {
        1,
        2,
        3
    };


    buffer.write(
        data,
        3);


    buffer.clear();


    EXPECT_EQ(
        buffer.size(),
        0);


    EXPECT_EQ(
        buffer.readableSize(),
        0);
}