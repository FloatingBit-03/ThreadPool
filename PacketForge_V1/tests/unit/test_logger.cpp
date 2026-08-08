#include <gtest/gtest.h>

#include "common/logger.hpp"

using packetforge::common::Logger;


TEST(LoggerTest, WritesInfoMessageToStdout)
{
    testing::internal::CaptureStdout();

    Logger::info(
        "Server started");

    const std::string output =
        testing::internal::GetCapturedStdout();

    EXPECT_EQ(
        output,
        "[INFO] Server started\n");
}


TEST(LoggerTest, WritesDebugMessageToStdout)
{
    testing::internal::CaptureStdout();

    Logger::debug(
        "Packet received");

    const std::string output =
        testing::internal::GetCapturedStdout();

    EXPECT_EQ(
        output,
        "[DEBUG] Packet received\n");
}


TEST(LoggerTest, WritesWarningMessageToStdout)
{
    testing::internal::CaptureStdout();

    Logger::warning(
        "Connection is slow");

    const std::string output =
        testing::internal::GetCapturedStdout();

    EXPECT_EQ(
        output,
        "[WARNING] Connection is slow\n");
}


TEST(LoggerTest, WritesErrorMessageToStderr)
{
    testing::internal::CaptureStderr();

    Logger::error(
        "Socket creation failed");

    const std::string output =
        testing::internal::GetCapturedStderr();

    EXPECT_EQ(
        output,
        "[ERROR] Socket creation failed\n");
}


TEST(LoggerTest, HandlesEmptyMessage)
{
    testing::internal::CaptureStdout();

    Logger::info(
        "");

    const std::string output =
        testing::internal::GetCapturedStdout();

    EXPECT_EQ(
        output,
        "[INFO] \n");
}