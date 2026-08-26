#pragma once

#include "common/error.hpp"
#include "protocol/opcode.hpp"
#include "protocol/packet.hpp"

namespace packetforge::protocol
{

class OpcodeDispatcher
{
public:

    [[nodiscard]]
    common::Error dispatch(
        const Packet& request,
        Packet& response
    ) const;


private:

    [[nodiscard]]
    common::Error handleHello(
        const Packet& request,
        Packet& response
    ) const;


    [[nodiscard]]
    common::Error makeErrorResponse(
        const Packet& request,
        ProtocolError error,
        Packet& response
    ) const;
};

} // namespace packetforge::protocol