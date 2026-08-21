#include "config/config.hpp"


namespace packetforge::config
{


// ----------------------------------------------------------
// Construction
// ----------------------------------------------------------

Config::Config()
    : values_{}
{
}


// ----------------------------------------------------------
// Contains
// ----------------------------------------------------------

bool Config::contains(
    const std::string& key
) const noexcept
{
    return values_.find(key) != values_.end();
}


// ----------------------------------------------------------
// Set
// ----------------------------------------------------------

void Config::set(
    const std::string& key,
    const ConfigValue& value
)
{
    values_[key] = value;
}


// ----------------------------------------------------------
// Get
// ----------------------------------------------------------

const ConfigValue* Config::get(
    const std::string& key
) const noexcept
{
    const auto it = values_.find(key);

    if (it == values_.end())
    {
        return nullptr;
    }

    return &it->second;
}

// ----------------------------------------------------------
// Clear
// ----------------------------------------------------------

void Config::clear() noexcept
{
    values_.clear();
}


} // namespace packetforge::config