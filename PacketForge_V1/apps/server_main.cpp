#include "server/server.hpp"

#include "network/connection.hpp"
#include "network/transport.hpp"

#include "protocol/packet.hpp"
#include "protocol/packet_io.hpp"
#include "protocol/opcode.hpp"

#include <cstdint>
#include <iostream>
#include <utility>

int main()
{
    using namespace packetforge;

    constexpr const char* SERVER_ADDRESS = "127.0.0.1";
    constexpr std::uint16_t SERVER_PORT = 9090;

    // ==========================================================
    // Create Server
    // ==========================================================

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


    // ==========================================================
    // Wait For Client
    // ==========================================================

    std::cout
        << "waiting for client connection..."
        << std::endl;


    network::Connection connection;


    const auto acceptError =
        server.accept(connection);

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


    // ==========================================================
    // Create Transport & Packet IO
    // ==========================================================

    network::Transport transport(
        std::move(connection)
    );

    protocol::PacketIO packetIO(
        transport
    );

    // ==========================================================
    // Receive Packet
    // ==========================================================

    std::cout
        << "\nWaiting for packet..."
        << std::endl;

    protocol::Packet packet;

    const auto receiveError =
        packetIO.receive(packet);

    if (!receiveError.ok())
    {
        std::cerr
            << "Failed to receive packet: "
            << receiveError.message()
            << '\n';

        transport.connection().disconnect();
        server.stop();

        return 1;
    }

    // ==========================================================
    // Display Packet
    // ==========================================================

    std::cout
        << "\nPacket received successfully\n";


    std::cout
        << "Magic: 0x"
        << std::hex
        << packet.magicNumber()
        << std::dec
        << '\n';


    std::cout
        << "Version: "
        << static_cast<int>(
            packet.version()
        )
        << '\n';


    std::cout
        << "Flags: "
        << static_cast<int>(
            packet.flags()
        )
        << '\n';


    std::cout
        << "Opcode: "
        << packet.opcode()
        << '\n';


    std::cout
        << "Sequence ID: "
        << packet.sequenceId()
        << '\n';


    std::cout
        << "Payload length: "
        << packet.payloadLength()
        << '\n';


    std::cout
        << "Payload: ";


    for (const auto byte : packet.payload())
    {
        std::cout
            << static_cast<char>(byte);
    }


    std::cout
        << '\n';

    
// ==========================================================
// Create Response Packet
// ==========================================================

protocol::Packet response;

response.setVersion(
    protocol::Packet::VERSION
);

response.setFlags(0);

response.setOpcode(
    static_cast<std::uint16_t>(
        protocol::Opcode::HelloResponse
    )
);

response.setSequenceId(
    packet.sequenceId()
);

response.setPayload(
    {
        'H',
        'e',
        'l',
        'l',
        'o',
        ' ',
        'f',
        'r',
        'o',
        'm',
        ' ',
        'P',
        'a',
        'c',
        'k',
        'e',
        't',
        'F',
        'o',
        'r',
        'g',
        'e',
        ' ',
        'S',
        'e',
        'r',
        'v',
        'e',
        'r'
    }
);


// ==========================================================
// Send Response Packet
// ==========================================================

std::cout
    << "\nSending response packet..."
    << std::endl;

const auto sendError =
    packetIO.send(response);

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

std::cout
    << "  Opcode: "
    << response.opcode()
    << '\n';

std::cout
    << "  Sequence ID: "
    << response.sequenceId()
    << '\n';

std::cout
    << "  Payload length: "
    << response.payloadLength()
    << '\n';

std::cout
    << "  Payload: ";

for (const auto byte : response.payload())
{
    std::cout
        << static_cast<char>(byte);
}

std::cout
    << '\n';

    // ==========================================================
    // Keep Connection Alive
    // ==========================================================

    std::cout
        << "\nPress ENTER to disconnect..."
        << std::flush;


    std::cin.get();


    // ==========================================================
    // Disconnect
    // ==========================================================

    transport.connection().disconnect();


    std::cout
        << "\nClient connection closed\n";


    // ==========================================================
    // Stop Server
    // ==========================================================

    server.stop();


    std::cout
        << "PacketForge server stopped successfully\n";


    return 0;
}