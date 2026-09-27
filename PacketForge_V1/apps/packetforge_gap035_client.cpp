#include "network/connection.hpp"
#include "network/transport.hpp"

#include "protocol/encoder.hpp"
#include "protocol/opcode.hpp"
#include "protocol/packet.hpp"

#include <cstdint>
#include <iostream>
#include <sys/socket.h>

namespace
{

constexpr const char* SERVER_ADDRESS = "127.0.0.1";
constexpr std::uint16_t SERVER_PORT = 9090;
constexpr std::uint32_t REQUEST_SEQUENCE_ID = 1;

const std::vector<std::uint8_t> REQUEST_PAYLOAD = {
    'H', 'e', 'l', 'l', 'o', ' ', 'P', 'a', 'c', 'k', 'e', 't',
    'F', 'o', 'r', 'g', 'e'
};

bool
configureAbortiveClose(
    packetforge::network::Transport& transport
)
{
    struct linger lingerOption
    {
        1,
        0
    };

    const int result =
        ::setsockopt(
            transport.connection().socket().nativeHandle(),
            SOL_SOCKET,
            SO_LINGER,
            &lingerOption,
            sizeof(lingerOption)
        );

    return result == 0;
}

} // namespace

int main()
{
    using namespace packetforge;

    std::cout
        << "GAP-035 Peer Disconnect During Workflow Client\n"
        << "Target: "
        << SERVER_ADDRESS
        << ":"
        << SERVER_PORT
        << "\n\n";

    // ------------------------------------------------------
    // GAP-035.1 — Connect to the real PacketForge server
    // ------------------------------------------------------

    network::Connection connection;

    const auto connectError =
        connection.connect(
            SERVER_ADDRESS,
            SERVER_PORT
        );

    if (!connectError.ok())
    {
        std::cerr
            << "Connection: FAIL - "
            << connectError.message()
            << '\n';

        return 1;
    }

    std::cout
        << "Connection: PASS\n"
        << "Connection state: "
        << (connection.isConnected()
                ? "CONNECTED"
                : "DISCONNECTED")
        << '\n';

    // Transport takes ownership of the established connection.
    network::Transport transport(
        std::move(connection)
    );

    // ------------------------------------------------------
    // GAP-035.2 — Build a valid HelloRequest
    // ------------------------------------------------------

    protocol::Packet request;

    request.setVersion(
        protocol::Packet::VERSION
    );

    request.setFlags(0);

    request.setOpcode(
        static_cast<std::uint16_t>(
            protocol::Opcode::HelloRequest
        )
    );

    request.setSequenceId(
        REQUEST_SEQUENCE_ID
    );

    request.setPayload(
        REQUEST_PAYLOAD
    );

    if (!request.isValid())
    {
        std::cerr
            << "HelloRequest validation: FAIL\n";

        return 1;
    }

    std::cout
        << "HelloRequest validation: PASS\n";

    // ------------------------------------------------------
    // GAP-035.3 — Send the request
    // ------------------------------------------------------

    protocol::Encoder encoder;

    std::vector<std::uint8_t> encodedRequest;

    try
    {
        encodedRequest =
            encoder.encode(request);
    }
    catch (const std::exception& error)
    {
        std::cerr
            << "HelloRequest encoding: FAIL - "
            << error.what()
            << '\n';

        return 1;
    }

    const auto sendError =
        transport.send(
            encodedRequest
        );

    if (!sendError.ok())
    {
        std::cerr
            << "HelloRequest transmission: FAIL - "
            << sendError.message()
            << '\n';

        return 1;
    }

    std::cout
        << "HelloRequest transmission: PASS\n";

    // ------------------------------------------------------
    // GAP-035 control point
    // ------------------------------------------------------
    //
    // At this point the server should have:
    //   1. received the request
    //   2. dispatched the request
    //   3. stopped at the GDB breakpoint immediately before
    //      packetIO.send(response)
    //
    // Do NOT press ENTER until GDB has stopped there.
    // ------------------------------------------------------

    std::cout
        << "\nWaiting for GDB breakpoint on the server...\n"
        << "When the server is stopped immediately before\n"
        << "packetIO.send(response), press ENTER here to\n"
        << "inject the peer TCP RST.\n"
        << std::endl;

    std::cin.get();

    // ------------------------------------------------------
    // GAP-035.6 — Configure abortive TCP close
    // ------------------------------------------------------

    if (!configureAbortiveClose(transport))
    {
        std::cerr
            << "TCP reset configuration: FAIL\n";

        return 1;
    }

    std::cout
        << "TCP reset configuration: PASS\n";

    // ------------------------------------------------------
    // GAP-035.6 — Trigger RST
    // ------------------------------------------------------

    transport.connection().disconnect();

    std::cout
        << "TCP reset injected: PASS\n"
        << "Connection state after reset: "
        << (transport.isConnected()
                ? "CONNECTED"
                : "DISCONNECTED")
        << '\n';

    std::cout
        << "GAP-035 peer disconnect injection: PASS\n";

    return 0;
}
