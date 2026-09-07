#include "server/server.hpp"

#include "network/connection.hpp"
#include "network/transport.hpp"

#include "protocol/packet.hpp"
#include "protocol/packet_io.hpp"
#include "protocol/opcode.hpp"
#include "protocol/opcode_dispatcher.hpp"

#include <cstdint>
#include <csignal>
#include <iostream>
#include <memory>
#include <utility>

namespace
{

volatile std::sig_atomic_t shutdownRequested = 0;

void handleShutdownSignal(int)
{
    // Signal handler must only record the shutdown request.
    shutdownRequested = 1;
}

bool isShutdownRequested() noexcept
{
    return shutdownRequested != 0;
}

} // namespace

int main()
{
    std::signal(SIGINT, handleShutdownSignal);

    using namespace packetforge;

    constexpr const char* SERVER_ADDRESS = "127.0.0.1";
    constexpr std::uint16_t SERVER_PORT = 9090;

    int exitCode = 0;

    server::Server server;

    /*
     * ========================================================
     * Application Execution Scope
     * ========================================================
     *
     * GAP-001.4:
     * Every application exit path transfers control to the
     * centralized cleanup section at the end of main().
     *
     * Transport is created only after a client connection has
     * been successfully accepted.
     *
     * std::unique_ptr is used instead of std::optional.
     * This avoids optional's in-place construction path while
     * still providing explicit ownership and lifetime management.
     * ========================================================
     */

    std::unique_ptr<network::Transport> transport;

    do
    {
        /*
         * ====================================================
         * Start Server
         * ====================================================
         */

        const auto startError =
            server.start(
                SERVER_ADDRESS,
                SERVER_PORT
            );

        if (!startError.ok())
        {
            std::cerr
                << "Failed to start PacketForge server: "
                << startError.message()
                << '\n';

            exitCode = 1;
            break;
        }

        std::cout
            << "PacketForge server started successfully\n";

        std::cout
            << "Listening on "
            << SERVER_ADDRESS
            << ':'
            << server.port()
            << '\n';

        std::cout
            << "Waiting for client connection..."
            << std::endl;

        network::Connection connection;

        /*
         * ====================================================
         * GAP-001.3 — Shutdown Check Before Accept
         * ====================================================
         */

        if (isShutdownRequested())
        {
            exitCode = 0;
            break;
        }

        /*
         * ====================================================
         * Accept Client Connection
         * ====================================================
         */

        const auto acceptError =
            server.accept(connection);

        /*
         * ====================================================
         * GAP-001.3 — Shutdown Check After Accept
         * ====================================================
         */

        if (isShutdownRequested())
        {
            exitCode = 0;
            break;
        }

        if (!acceptError.ok())
        {
            std::cerr
                << "Failed to accept client connection: "
                << acceptError.message()
                << '\n';

            exitCode = 1;
            break;
        }

        std::cout
            << "Client connected successfully\n";

        std::cout
            << "Connection state: "
            << (
                connection.isConnected()
                    ? "CONNECTED"
                    : "DISCONNECTED"
            )
            << '\n';

        /*
         * ====================================================
         * Create Transport
         * ====================================================
         *
         * Ownership of the accepted Connection is transferred
         * to Transport.
         * ====================================================
         */

        transport.reset(
            new network::Transport(
                std::move(connection)
            )
        );

        protocol::PacketIO packetIO(
            *transport
        );

        protocol::OpcodeDispatcher dispatcher;

        /*
         * ====================================================
         * Receive Request
         * ====================================================
         */

        std::cout
            << "\nWaiting for packet..."
            << std::endl;

        protocol::Packet request;

        protocol::ProtocolError protocolError =
            protocol::ProtocolError::InvalidPacket;

        const auto receiveError =
            packetIO.receive(
                request,
                &protocolError
            );

        /*
         * ====================================================
         * GAP-001.3 — Shutdown Check After Receive
         * ====================================================
         */

        if (isShutdownRequested())
        {
            exitCode = 0;
            break;
        }

        /*
         * ====================================================
         * Protocol / Receive Error
         * ====================================================
         */

        if (!receiveError.ok())
        {
            std::cerr
                << "Failed to receive packet: "
                << receiveError.message()
                << '\n';

            if (
                receiveError.code()
                ==
                common::ErrorCode::ProtocolError
            )
            {
                std::cerr
                    << "Protocol error detected\n";

                /*
                 * --------------------------------------------
                 * Construct Protocol Error Response
                 * --------------------------------------------
                 */

                protocol::Packet response;

                response.setOpcode(
                    static_cast<std::uint16_t>(
                        protocol::Opcode::ErrorResponse
                    )
                );

                response.setSequenceId(0);

                response.setPayload(
                    {
                        static_cast<std::uint8_t>(
                            protocolError
                        )
                    }
                );

                /*
                 * --------------------------------------------
                 * Send Protocol Error Response
                 * --------------------------------------------
                 */

                const auto sendError =
                    packetIO.send(response);

                if (!sendError.ok())
                {
                    std::cerr
                        << "Failed to send protocol error response: "
                        << sendError.message()
                        << '\n';

                    exitCode = 1;
                    break;
                }

                /*
                 * --------------------------------------------
                 * GAP-001.3 — Shutdown Check After Send
                 * --------------------------------------------
                 */

                if (isShutdownRequested())
                {
                    exitCode = 0;
                    break;
                }

                std::cout
                    << "Protocol error response sent successfully\n";
            }

            exitCode = 1;
            break;
        }

        std::cout
            << "Packet received successfully\n";

        /*
         * ====================================================
         * Dispatch Request
         * ====================================================
         */

        std::cout
            << "\nDispatching request..."
            << std::endl;

        protocol::Packet response;

        const auto dispatchError =
            dispatcher.dispatch(
                request,
                response
            );

        /*
         * ====================================================
         * GAP-001.3 — Shutdown Check After Dispatch
         * ====================================================
         */

        if (isShutdownRequested())
        {
            exitCode = 0;
            break;
        }

        if (!dispatchError.ok())
        {
            std::cerr
                << "Failed to dispatch packet: "
                << dispatchError.message()
                << '\n';

            exitCode = 1;
            break;
        }

        std::cout
            << "Request dispatched successfully\n";

        /*
         * ====================================================
         * Send Response
         * ====================================================
         */

        std::cout
            << "Sending response packet..."
            << std::endl;

        const auto sendError =
            packetIO.send(response);

        if (!sendError.ok())
        {
            std::cerr
                << "Failed to send response packet: "
                << sendError.message()
                << '\n';

            exitCode = 1;
            break;
        }

        /*
         * ====================================================
         * GAP-001.3 — Shutdown Check After Send
         * ====================================================
         */

        if (isShutdownRequested())
        {
            exitCode = 0;
            break;
        }

        std::cout
            << "Response packet sent successfully\n";

        /*
         * ====================================================
         * Keep Connection Alive
         * ====================================================
         */

        std::cout
            << "\nPress ENTER to disconnect..."
            << std::flush;

        std::cin.get();

        exitCode = 0;

    } while (false);

    /*
     * ========================================================
     * GAP-001.4 — CENTRALIZED CLEANUP
     * ========================================================
     *
     * Every exit path from the application execution scope
     * reaches this section.
     *
     * Cleanup order:
     *
     *     1. Disconnect active client connection
     *     2. Stop listening server
     *
     * Transport owns the accepted Connection after successful
     * connection establishment.
     *
     * Connection::disconnect() and Server::stop() are
     * idempotent, so cleanup is safe.
     * ========================================================
     */

    if (transport)
    {
        transport->connection().disconnect();

        std::cout
            << "Client connection closed\n";
    }

    transport.reset();

    if (server.isRunning())
    {
        server.stop();

        std::cout
            << "PacketForge server stopped successfully\n";
    }

    return exitCode;
}
