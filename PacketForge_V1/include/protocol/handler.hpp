#pragma once

#include "common/error.hpp"
#include "protocol/packet.hpp"

namespace packetforge::protocol
{

class Handler
{
public:

    common::Error handleHello(
        const Packet& request,
        Packet& response
    ) const;
};

} // namespace packetforge::protocol