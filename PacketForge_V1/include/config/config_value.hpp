#pragma once

#include <cstdint>
#include <string>
#include <variant>

namespace packetforge::config
{

class ConfigValue
{
public:

    // ------------------------------------------------------
    // Types
    // ------------------------------------------------------

    using Value =
        std::variant<
            std::string,
            std::int64_t,
            bool
        >;


    // ------------------------------------------------------
    // Construction & Destruction
    // ------------------------------------------------------

    ConfigValue();

    ConfigValue(
        const std::string& value
    );

    ConfigValue(
        const char* value
    );

    ConfigValue(
        std::int64_t value
    );

    ConfigValue(
        bool value
    );

    ~ConfigValue() = default;


    // ------------------------------------------------------
    // Copy
    // ------------------------------------------------------

    ConfigValue(
        const ConfigValue&
    ) = default;

    ConfigValue& operator=(
        const ConfigValue&
    ) = default;


    // ------------------------------------------------------
    // Move
    // ------------------------------------------------------

    ConfigValue(
        ConfigValue&&
    ) noexcept = default;

    ConfigValue& operator=(
        ConfigValue&&
    ) noexcept = default;


    // ------------------------------------------------------
    // Type Inspection
    // ------------------------------------------------------

    bool isString() const noexcept;

    bool isInteger() const noexcept;

    bool isBoolean() const noexcept;


    // ------------------------------------------------------
    // Value Access
    // ------------------------------------------------------

    const std::string& asString() const;

    std::int64_t asInteger() const;

    bool asBoolean() const;


private:

    Value value_;
};

} // namespace packetforge::config