#include "protocol/opcode_dispatcher.hpp"

#include "protocol/opcode.hpp"

namespace packetforge::protocol
{

common::Error
OpcodeDispatcher::dispatch(
    const Packet& request,
    Packet& response
) const
{
    // ------------------------------------------------------
    // Validate protocol version
    // ------------------------------------------------------

    if (request.version() != Packet::VERSION)
    {
        return makeErrorResponse(
            request,
            ProtocolError::UnsupportedVersion,
            response
        );
    }


    // ------------------------------------------------------
    // Dispatch opcode
    // ------------------------------------------------------

    switch (
        static_cast<Opcode>(
            request.opcode()
        )
    )
    {
        case Opcode::HelloRequest:

            return handleHello(
                request,
                response
            );

        default:

            return makeErrorResponse(
                request,
                ProtocolError::UnsupportedOpcode,
                response
            );
    }
}


// ==========================================================
// Hello Request Handler
// ==========================================================

common::Error
OpcodeDispatcher::handleHello(
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


// ==========================================================
// Protocol Error Response
// ==========================================================

common::Error
OpcodeDispatcher::makeErrorResponse(
    const Packet& request,
    ProtocolError error,
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
            Opcode::ErrorResponse
        )
    );

    response.setSequenceId(
        request.sequenceId()
    );

    response.setPayload(
        {
            static_cast<std::uint8_t>(
                error
            )
        }
    );

    return common::Error(
        common::ErrorCode::Success,
        ""
    );
}

} // namespace packetforge::protocol