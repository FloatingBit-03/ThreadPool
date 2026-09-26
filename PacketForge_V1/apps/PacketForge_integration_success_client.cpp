#include "client/client.hpp"

#include "protocol/opcode.hpp"
#include "protocol/packet.hpp"

#include <cstdint>
#include <iostream>
#include <vector>

namespace
{

constexpr const char* SERVER_ADDRESS = "127.0.0.1";
constexpr std::uint16_t SERVER_PORT = 9090;

constexpr std::uint32_t REQUEST_SEQUENCE_ID = 1;

const std::vector<std::uint8_t> REQUEST_PAYLOAD =
{
    'H', 'e', 'l', 'l', 'o', ' ',
    'P', 'a', 'c', 'k', 'e', 't',
    'F', 'o', 'r', 'g', 'e'
};

const std::vector<std::uint8_t> EXPECTED_RESPONSE_PAYLOAD =
{
    'H', 'e', 'l', 'l', 'o', ' ',
    'f', 'r', 'o', 'm', ' ',
    'P', 'a', 'c', 'k', 'e', 't',
    'F', 'o', 'r', 'g', 'e', ' ',
    'S', 'e', 'r', 'v', 'e', 'r'
};

int fail(
    packetforge::client::Client& client,
    const char* message)
{
    std::cerr
        << "FAIL: "
        << message
        << '\n';

    client.disconnect();

    return 1;
}

} // namespace

int main()
{
    using packetforge::client::Client;
    using packetforge::protocol::Opcode;
    using packetforge::protocol::Packet;

    std::cout
        << "GAP-022 Successful Workflow Test\n";

    Client client;

    // ------------------------------------------------------
    // 022.1 / 022.2 — Connect
    // ------------------------------------------------------

    const auto connectError =
        client.connect(
            SERVER_ADDRESS,
            SERVER_PORT
        );

    if (!connectError.ok())
    {
        return fail(
            client,
            "Client connection failed"
        );
    }

    if (!client.isConnected())
    {
        return fail(
            client,
            "Client connection state is not CONNECTED"
        );
    }

    std::cout
        << "Connect: PASS\n";

    // ------------------------------------------------------
    // 022.3 — Create valid HelloRequest
    // ------------------------------------------------------

    Packet request;

    request.setVersion(Packet::VERSION);
    request.setFlags(0);
    request.setOpcode(
        static_cast<std::uint16_t>(
            Opcode::HelloRequest
        )
    );
    request.setSequenceId(
        REQUEST_SEQUENCE_ID
    );
    request.setPayload(REQUEST_PAYLOAD);

    if (!request.isValid())
    {
        return fail(
            client,
            "HelloRequest validation failed"
        );
    }

    if (request.opcode() !=
        static_cast<std::uint16_t>(Opcode::HelloRequest))
    {
        return fail(
            client,
            "HelloRequest opcode is incorrect"
        );
    }

    if (request.sequenceId() != REQUEST_SEQUENCE_ID)
    {
        return fail(
            client,
            "HelloRequest sequence ID is incorrect"
        );
    }

    if (request.payload() != REQUEST_PAYLOAD)
    {
        return fail(
            client,
            "HelloRequest payload is incorrect"
        );
    }

    std::cout
        << "HelloRequest construction: PASS\n";

    // ------------------------------------------------------
    // 022.4 — Send request
    // ------------------------------------------------------

    const auto sendError =
        client.send(request);

    if (!sendError.ok())
    {
        return fail(
            client,
            "HelloRequest transmission failed"
        );
    }

    std::cout
        << "Request transmission: PASS\n";

    // ------------------------------------------------------
    // 022.5 — Receive HelloResponse
    // ------------------------------------------------------

    Packet response;

    const auto receiveError =
        client.receive(response);

    if (!receiveError.ok())
    {
        return fail(
            client,
            "HelloResponse reception failed"
        );
    }

    std::cout
        << "HelloResponse received: PASS\n";

    // ------------------------------------------------------
    // 022.6 — Validate Magic
    // ------------------------------------------------------

    if (response.magicNumber() != Packet::MagicNumber)
    {
        return fail(
            client,
            "Response magic number is incorrect"
        );
    }

    std::cout
        << "Magic validation: PASS\n";

    // ------------------------------------------------------
    // 022.7 — Validate Version
    // ------------------------------------------------------

    if (response.version() != Packet::VERSION)
    {
        return fail(
            client,
            "Response version is incorrect"
        );
    }

    std::cout
        << "Version validation: PASS\n";

    // ------------------------------------------------------
    // 022.8 — Validate Opcode
    // ------------------------------------------------------

    if (response.opcode() !=
        static_cast<std::uint16_t>(Opcode::HelloResponse))
    {
        return fail(
            client,
            "Response opcode is not HelloResponse"
        );
    }

    std::cout
        << "Opcode validation: PASS\n";

    // ------------------------------------------------------
    // Validate flags as part of response semantics
    // ------------------------------------------------------

    if (response.flags() != 0)
    {
        return fail(
            client,
            "Response flags are incorrect"
        );
    }

    std::cout
        << "Flags validation: PASS\n";

    // ------------------------------------------------------
    // 022.9 — Validate sequence correlation
    // ------------------------------------------------------

    if (response.sequenceId() != REQUEST_SEQUENCE_ID)
    {
        return fail(
            client,
            "Response sequence ID does not match request"
        );
    }

    std::cout
        << "Sequence ID validation: PASS\n";

    // ------------------------------------------------------
    // 022.10 — Validate payload
    // ------------------------------------------------------

    if (response.payload() != EXPECTED_RESPONSE_PAYLOAD)
    {
        return fail(
            client,
            "Response payload is incorrect"
        );
    }

    if (response.payloadLength() !=
        EXPECTED_RESPONSE_PAYLOAD.size())
    {
        return fail(
            client,
            "Response payload length is incorrect"
        );
    }

    std::cout
        << "Payload validation: PASS\n";

    // ------------------------------------------------------
    // Validate final response structure
    // ------------------------------------------------------

    if (!response.isValid())
    {
        return fail(
            client,
            "Final HelloResponse validation failed"
        );
    }

    // ------------------------------------------------------
    // 022.11 — Disconnect
    // ------------------------------------------------------

    client.disconnect();

    if (client.isConnected())
    {
        return fail(
            client,
            "Client remained CONNECTED after disconnect"
        );
    }

    std::cout
        << "Disconnect: PASS\n";

    // ------------------------------------------------------
    // 022.12 — Complete integration test
    // ------------------------------------------------------

    std::cout
        << "\nGAP-022 successful workflow test: PASS\n";

    return 0;
}
