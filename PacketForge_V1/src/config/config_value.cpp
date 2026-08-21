#include "config/config_value.hpp"

#include <stdexcept>

namespace packetforge::config
{

// ------------------------------------------------------
// Construction
// ------------------------------------------------------

ConfigValue::ConfigValue()
    : value_(std::string{})
{
}

ConfigValue::ConfigValue(const std::string& value)
    : value_(value)
{
}

ConfigValue::ConfigValue(const char* value)
    : value_(value)
{
}

ConfigValue::ConfigValue(std::int64_t value)
    : value_(value)
{
}

ConfigValue::ConfigValue(bool value)
    : value_(value)
{
}


// ------------------------------------------------------
// Type Inspection
// ------------------------------------------------------

bool ConfigValue::isString() const noexcept
{
    return std::holds_alternative<std::string>(value_);
}

bool ConfigValue::isInteger() const noexcept
{
    return std::holds_alternative<std::int64_t>(value_);
}

bool ConfigValue::isBoolean() const noexcept
{
    return std::holds_alternative<bool>(value_);
}


// ------------------------------------------------------
// Value Access
// ------------------------------------------------------

const std::string& ConfigValue::asString() const
{
    if (!isString())
    {
        throw std::runtime_error(
            "ConfigValue does not contain a string"
        );
    }

    return std::get<std::string>(value_);
}

std::int64_t ConfigValue::asInteger() const
{
    if (!isInteger())
    {
        throw std::runtime_error(
            "ConfigValue does not contain an integer"
        );
    }

    return std::get<std::int64_t>(value_);
}

bool ConfigValue::asBoolean() const
{
    if (!isBoolean())
    {
        throw std::runtime_error(
            "ConfigValue does not contain a boolean"
        );
    }

    return std::get<bool>(value_);
}

} // namespace packetforge::config