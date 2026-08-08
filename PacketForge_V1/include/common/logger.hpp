#pragma once

#include <string>

namespace packetforge::common
{

enum class LogLevel
{
    Debug,
    Info,
    Warning,
    Error
};


class Logger
{
public:

    static void debug(
        const std::string& message);

    static void info(
        const std::string& message);

    static void warning(
        const std::string& message);

    static void error(
        const std::string& message);

private:

    static void log(
        LogLevel level,
        const std::string& message);

    static const char* toString(
        LogLevel level) noexcept;
};

}