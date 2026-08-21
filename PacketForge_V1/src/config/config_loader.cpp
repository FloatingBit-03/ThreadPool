#include "config/config_loader.hpp"

#include <cctype>
#include <fstream>
#include <stdexcept>
#include <string>

namespace packetforge::config
{

namespace
{

// ----------------------------------------------------------
// Trim whitespace from both ends
// ----------------------------------------------------------

std::string trim(
    const std::string& value
)
{
    std::size_t begin = 0;

    while (
        begin < value.size() &&
        std::isspace(
            static_cast<unsigned char>(value[begin])
        )
    )
    {
        ++begin;
    }

    std::size_t end = value.size();

    while (
        end > begin &&
        std::isspace(
            static_cast<unsigned char>(value[end - 1])
        )
    )
    {
        --end;
    }

    return value.substr(
        begin,
        end - begin
    );
}


// ----------------------------------------------------------
// Parse a ConfigValue
// ----------------------------------------------------------

ConfigValue parseValue(
    const std::string& text
)
{
    const std::string value = trim(text);

    if (value.empty())
    {
        throw std::runtime_error(
            "Configuration value cannot be empty"
        );
    }

    // Boolean
    if (value == "true")
    {
        return ConfigValue(true);
    }

    if (value == "false")
    {
        return ConfigValue(false);
    }

    // Integer
    try
    {
        std::size_t position = 0;

        const long long integerValue =
            std::stoll(value, &position);

        if (position == value.size())
        {
            return ConfigValue(
                static_cast<std::int64_t>(
                    integerValue
                )
            );
        }
    }
    catch (const std::exception&)
    {
        // Not an integer.
        // Treat it as a string below.
    }

    // String
    return ConfigValue(value);
}

} // namespace


// ----------------------------------------------------------
// Loading
// ----------------------------------------------------------

Config ConfigLoader::load(
    const std::string& path
) const
{
    std::ifstream file(path);

    if (!file.is_open())
    {
        throw std::runtime_error(
            "Failed to open configuration file: " +
            path
        );
    }

    Config config;

    std::string line;
    std::size_t lineNumber = 0;

    while (std::getline(file, line))
    {
        ++lineNumber;

        const std::string trimmedLine =
            trim(line);

        // Ignore empty lines.
        if (trimmedLine.empty())
        {
            continue;
        }

        // Ignore comments.
        if (trimmedLine[0] == '#')
        {
            continue;
        }

        // Find key/value separator.
        const std::size_t separator =
            trimmedLine.find('=');

        if (separator == std::string::npos)
        {
            throw std::runtime_error(
                "Malformed configuration at line " +
                std::to_string(lineNumber)
            );
        }

        const std::string key =
            trim(
                trimmedLine.substr(
                    0,
                    separator
                )
            );

        const std::string value =
            trim(
                trimmedLine.substr(
                    separator + 1
                )
            );

        if (key.empty())
        {
            throw std::runtime_error(
                "Configuration key cannot be empty "
                "at line " +
                std::to_string(lineNumber)
            );
        }

        if (value.empty())
        {
            throw std::runtime_error(
                "Configuration value cannot be empty "
                "at line " +
                std::to_string(lineNumber)
            );
        }

        config.set(
            key,
            parseValue(value)
        );
    }

    return config;
}

} // namespace packetforge::config