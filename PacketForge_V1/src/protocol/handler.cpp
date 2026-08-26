#include "protocol/handler.hpp"

#include "protocol/opcode.hpp"

#include <cstdint>

namespace packetforge::protocol
{

common::Error
Handler::handleHello(
    const Packet& request,
    Packet& response
) const
{
    response = Packet{};

    response.setVersion(
        Packet::VERSION
    );

    response.setFlags(0);

    response.setOpcode(
        static_cast<std::uint16_t>(
            Opcode::HelloResponse
        )
    );

    response.setSequenceId(
        request.sequenceId()
    );

    response.setPayload(
        {
            'H', 'e', 'l', 'l', 'o', ' ',
            'f', 'r', 'o', 'm', ' ',
            'P', 'a', 'c', 'k', 'e', 't',
            'F', 'o', 'r', 'g', 'e', ' ',
            'S', 'e', 'r', 'v', 'e', 'r'
        }
    );

    return common::Error(
        common::ErrorCode::Success,
        ""
    );
}

} // namespace packetforge::protocol