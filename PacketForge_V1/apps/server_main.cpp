#include "server/server.hpp"

#include "network/connection.hpp"
#include "network/transport.hpp"

#include "protocol/packet.hpp"
#include "protocol/packet_io.hpp"
#include "protocol/opcode.hpp"
#include "protocol/opcode_dispatcher.hpp"

#include <cstdint>
#include <iostream>
#include <utility>

int main()
{
    using namespace packetforge;

    constexpr const char* SERVER_ADDRESS = "127.0.0.1";
    constexpr std::uint16_t SERVER_PORT = 9090;

    server::Server server;

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

        return 1;
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

    const auto acceptError =
        server.accept(
            connection
        );

    if (!acceptError.ok())
    {
        std::cerr
            << "Failed to accept client connection: "
            << acceptError.message()
            << '\n';

        server.stop();

        return 1;
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


    network::Transport transport(
        std::move(connection)
    );

    protocol::PacketIO packetIO(
        transport
    );

    protocol::OpcodeDispatcher dispatcher;


    // ======================================================
    // Receive Request
    // ======================================================

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


    // ======================================================
    // Protocol Error
    // ======================================================

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


            // ------------------------------------------------
            // Construct protocol error response
            // ------------------------------------------------

            protocol::Packet response;

            response.setOpcode(
                static_cast<std::uint16_t>(
                    protocol::Opcode::ErrorResponse
                )
            );

            response.setSequenceId(
                0
            );

            response.setPayload(
                {
                    static_cast<std::uint8_t>(
                        protocolError
                    )
                }
            );


            // ------------------------------------------------
            // Send protocol error response
            // ------------------------------------------------

            const auto sendError =
                packetIO.send(
                    response
                );

            if (!sendError.ok())
            {
                std::cerr
                    << "Failed to send protocol error response: "
                    << sendError.message()
                    << '\n';
            }
            else
            {
                std::cout
                    << "Protocol error response sent successfully\n";
            }
        }


        transport.connection().disconnect();
        server.stop();

        return 1;
    }


    std::cout
        << "Packet received successfully\n";


    // ======================================================
    // Dispatch Request
    // ======================================================

    std::cout
        << "\nDispatching request..."
        << std::endl;

    protocol::Packet response;

    const auto dispatchError =
        dispatcher.dispatch(
            request,
            response
        );

    if (!dispatchError.ok())
    {
        std::cerr
            << "Failed to dispatch packet: "
            << dispatchError.message()
            << '\n';

        transport.connection().disconnect();
        server.stop();

        return 1;
    }

    std::cout
        << "Request dispatched successfully\n";


    // ======================================================
    // Send Response
    // ======================================================

    std::cout
        << "Sending response packet..."
        << std::endl;

    const auto sendError =
        packetIO.send(
            response
        );

    if (!sendError.ok())
    {
        std::cerr
            << "Failed to send response packet: "
            << sendError.message()
            << '\n';

        transport.connection().disconnect();
        server.stop();

        return 1;
    }

    std::cout
        << "Response packet sent successfully\n";


    // ======================================================
    // Keep Connection Alive
    // ======================================================

    std::cout
        << "\nPress ENTER to disconnect..."
        << std::flush;

    std::cin.get();


    transport.connection().disconnect();

    std::cout
        << "\nClient connection closed\n";

    server.stop();

    std::cout
        << "PacketForge server stopped successfully\n";

    return 0;
}