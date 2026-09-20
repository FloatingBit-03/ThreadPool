#include "client/client.hpp"
#include "protocol/opcode.hpp"
#include "protocol/packet.hpp"

#include <cstdint>
#include <iostream>

int main()
{
    packetforge::client::Client client;

    const auto connectError =
        client.connect("127.0.0.1", 9090);

    if (!connectError.ok())
    {
        std::cerr
            << "Failed to connect: "
            << connectError.message()
            << '\n';

        return 1;
    }

    std::cout
        << "Client connected successfully\n";

    std::cout
        << "Connection state: "
        << (client.isConnected()
                ? "CONNECTED"
                : "DISCONNECTED")
        << '\n';

    // ------------------------------------------------------------
    // Send a valid request so the server completes its normal
    // receive -> dispatch -> send path and reaches its wait state.
    // ------------------------------------------------------------

    packetforge::protocol::Packet request;

    request.setVersion(
        packetforge::protocol::Packet::VERSION
    );

    request.setFlags(0);

    request.setOpcode(
        static_cast<std::uint16_t>(
            packetforge::protocol::Opcode::HelloRequest
        )
    );

    request.setSequenceId(1);

    const auto sendError =
        client.send(request);

    if (!sendError.ok())
    {
        std::cerr
            << "Failed to send request: "
            << sendError.message()
            << '\n';

        return 1;
    }

    std::cout
        << "HelloRequest sent successfully\n";

    // ------------------------------------------------------------
    // Receive the normal server response.
    // ------------------------------------------------------------

    packetforge::protocol::Packet response;

    const auto receiveError =
        client.receive(response);

    if (!receiveError.ok())
    {
        std::cerr
            << "Failed to receive response: "
            << receiveError.message()
            << '\n';

        return 1;
    }

    std::cout
        << "Initial server response received successfully\n";

    // ------------------------------------------------------------
    // Important:
    // Do NOT call client.disconnect().
    //
    // Wait for the SERVER to close the connection.
    // ------------------------------------------------------------

    std::cout
        << "\nWaiting for server to close connection...\n";

    packetforge::protocol::Packet secondPacket;

    const auto remoteCloseError =
        client.receive(secondPacket);

    std::cout
        << "Receive after server closure: "
        << remoteCloseError.message()
        << '\n';

    std::cout
        << "Connection state after server closure: "
        << (client.isConnected()
                ? "CONNECTED"
                : "DISCONNECTED")
        << '\n';
    
    std::cout
    << "\nTesting send after remote closure...\n";

    packetforge::protocol::Packet postClosePacket;

    postClosePacket.setVersion(
        packetforge::protocol::Packet::VERSION
    );

    postClosePacket.setFlags(0);

    postClosePacket.setOpcode(
        static_cast<std::uint16_t>(
            packetforge::protocol::Opcode::HelloRequest
        )
    );

    postClosePacket.setSequenceId(2);

    const auto sendAfterRemoteCloseError =
        client.send(postClosePacket);

    std::cout
        << "Send after remote closure result: "
        << sendAfterRemoteCloseError.message()
        << '\n';

    std::cout
        << "Connection state after send attempt: "
        << (client.isConnected()
                ? "CONNECTED"
                : "DISCONNECTED")
        << '\n';


    std::cout
        << "\nTesting receive after remote closure...\n";

    packetforge::protocol::Packet receiveAfterRemoteClosePacket;

    const auto receiveAfterRemoteCloseError =
        client.receive(receiveAfterRemoteClosePacket);

    std::cout
        << "Receive after remote closure result: "
        << receiveAfterRemoteCloseError.message()
        << '\n';

    std::cout
        << "Connection state after receive attempt: "
        << (client.isConnected()
                ? "CONNECTED"
                : "DISCONNECTED")
        << '\n';

    std::cout
    << "\nTesting explicit disconnect after remote closure...\n";

    client.disconnect();

    std::cout
        << "Explicit disconnect completed successfully\n";

    std::cout
        << "Connection state after explicit disconnect: "
        << (client.isConnected()
                ? "CONNECTED"
                : "DISCONNECTED")
        << '\n';

    return 0;
}