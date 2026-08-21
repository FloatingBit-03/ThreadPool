#pragma once

#include <string>
#include <unordered_map>

#include "config/config_value.hpp"

namespace packetforge::config
{

class Config
{
public:

    Config();

    bool contains(
        const std::string& key
    ) const noexcept;

    void set(
        const std::string& key,
        const ConfigValue& value
    );

    const ConfigValue* get(
        const std::string& key
    ) const noexcept;

    void clear() noexcept;

private:

    std::unordered_map<
        std::string,
        ConfigValue
    > values_;
};

} // namespace packetforge::config