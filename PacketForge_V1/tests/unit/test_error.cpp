#include <gtest/gtest.h>

#include "common/error.hpp"


using packetforge::common::Error;
using packetforge::common::ErrorCode;


TEST(ErrorTest, ConstructsErrorWithCodeAndMessage)
{
    Error error(
        ErrorCode::SocketError,
        "socket creation failed");


    EXPECT_EQ(
        error.code(),
        ErrorCode::SocketError);


    EXPECT_EQ(
        error.message(),
        "socket creation failed");


    EXPECT_FALSE(error.ok());
}



TEST(ErrorTest, SuccessErrorReturnsTrueFromOk)
{
    Error error(
        ErrorCode::Success,
        "");


    EXPECT_TRUE(error.ok());
}


TEST(ErrorTest, PreservesErrorMessage)
{
    Error error(
        ErrorCode::InvalidPacket,
        "Malformed packet received");


    EXPECT_EQ(
        error.code(),
        ErrorCode::InvalidPacket);


    EXPECT_EQ(
        error.message(),
        "Malformed packet received");
}


TEST(ErrorTest, DefaultFailureIsNotSuccess)
{
    Error error(
        ErrorCode::UnknownError,
        "unknown failure");


    EXPECT_NE(
        error.code(),
        ErrorCode::Success);


    EXPECT_FALSE(
        error.ok());
}