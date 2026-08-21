#pragma once

#include <string>

#include "config/config.hpp"

namespace packetforge::config
{

class ConfigLoader
{
public:

    // ------------------------------------------------------
    // Construction
    // ------------------------------------------------------

    ConfigLoader() = default;

    // ------------------------------------------------------
    // Loading
    // ------------------------------------------------------

    Config load(
        const std::string& path
    ) const;

};

} // namespace packetforge::config