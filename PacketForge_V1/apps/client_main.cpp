#include "client/client.hpp"
#include "protocol/opcode.hpp"
#include "protocol/packet.hpp"

#include <cstdint>
#include <iostream>
#include <vector>

int main()
{
    packetforge::client::Client client;

    // ------------------------------------------------------
    // Connect to PacketForge server
    // ------------------------------------------------------

    std::cout
        << "Connecting to PacketForge server...\n";

    const auto connectError =
        client.connect(
            "127.0.0.1",
            9090
        );

    if (!connectError.ok())
    {
        std::cerr
            << "Failed to connect to PacketForge server: "
            << connectError.message()
            << '\n';

        return 1;
    }

    std::cout
        << "PacketForge client connected successfully\n";

    std::cout
        << "Connection state: "
        << (client.isConnected()
                ? "CONNECTED"
                : "DISCONNECTED")
        << '\n';


    // ------------------------------------------------------
    // Create PacketForge packet
    // ------------------------------------------------------

    packetforge::protocol::Packet packet;

    packet.setVersion(
        packetforge::protocol::Packet::VERSION
    );

    packet.setFlags(0);

    packet.setOpcode(
    static_cast<std::uint16_t>(
        packetforge::protocol::Opcode::HelloRequest
        )
    );

    packet.setSequenceId(1);

    const std::vector<std::uint8_t> payload =
    {
        'H',
        'e',
        'l',
        'l',
        'o',
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
        'e'
    };

    packet.setPayload(payload);


    // ------------------------------------------------------
    // Validate packet
    // ------------------------------------------------------

    if (!packet.isValid())
    {
        std::cerr
            << "Packet validation failed\n";

        client.disconnect();

        return 1;
    }

    std::cout
        << "Packet created successfully\n";

    std::cout
        << "  Opcode: "
        << packet.opcode()
        << '\n';

    std::cout
        << "  Sequence ID: "
        << packet.sequenceId()
        << '\n';

    std::cout
        << "  Payload length: "
        << packet.payloadLength()
        << '\n';


    // ------------------------------------------------------
    // Send packet
    // ------------------------------------------------------

    std::cout
        << "Sending PacketForge packet...\n";

    const auto sendError =
        client.send(packet);

    if (!sendError.ok())
    {
        std::cerr
            << "Failed to send packet: "
            << sendError.message()
            << '\n';

        client.disconnect();

        return 1;
    }

    std::cout
    << "Packet sent successfully\n";


    // ------------------------------------------------------
    // Receive Response
    // ------------------------------------------------------

    std::cout
        << "Waiting for server response...\n";

    packetforge::protocol::Packet response;

    const auto receiveError =
        client.receive(response);

    if (!receiveError.ok())
    {
        std::cerr
            << "Failed to receive server response: "
            << receiveError.message()
            << '\n';

        client.disconnect();

        return 1;
    }

    std::cout
        << "\nServer response received successfully\n";

    std::cout
        << "  Magic: 0x"
        << std::hex
        << response.magicNumber()
        << std::dec
        << '\n';

    std::cout
        << "  Version: "
        << static_cast<int>(
            response.version()
        )
        << '\n';

    std::cout
        << "  Flags: "
        << static_cast<int>(
            response.flags()
        )
        << '\n';

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


    // ------------------------------------------------------
    // Keep connection alive
    // ------------------------------------------------------

    std::cout
        << "Press ENTER to disconnect...\n";

    std::cin.get();


    // ------------------------------------------------------
    // Disconnect
    // ------------------------------------------------------

    client.disconnect();

    std::cout
        << "PacketForge client disconnected successfully\n";

    return 0;
}