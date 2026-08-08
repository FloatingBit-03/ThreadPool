#include "common/logger.hpp"

#include <iostream>

namespace packetforge::common
{

void Logger::debug(
    const std::string& message)
{
    log(
        LogLevel::Debug,
        message);
}


void Logger::info(
    const std::string& message)
{
    log(
        LogLevel::Info,
        message);
}


void Logger::warning(
    const std::string& message)
{
    log(
        LogLevel::Warning,
        message);
}


void Logger::error(
    const std::string& message)
{
    log(
        LogLevel::Error,
        message);
}


void Logger::log(
    LogLevel level,
    const std::string& message)
{
    std::ostream& stream =
        (level == LogLevel::Error)
            ? std::cerr
            : std::cout;

    stream
        << '['
        << toString(level)
        << "] "
        << message
        << '\n';
}


const char* Logger::toString(
    LogLevel level) noexcept
{
    switch (level)
    {
        case LogLevel::Debug:
            return "DEBUG";

        case LogLevel::Info:
            return "INFO";

        case LogLevel::Warning:
            return "WARNING";

        case LogLevel::Error:
            return "ERROR";
    }

    return "UNKNOWN";
}

}