#include "network/connection.hpp"

#include <cstdint>
#include <iostream>

int main()
{
    using namespace packetforge;

    constexpr const char* SERVER_ADDRESS = "127.0.0.1";
    constexpr std::uint16_t SERVER_PORT = 9090;

    std::cout
        << "PacketForge shutdown test client\n"
        << "Connecting to PacketForge server..."
        << std::endl;

    network::Connection connection;

    const auto connectError =
        connection.connect(SERVER_ADDRESS, SERVER_PORT);

    if (!connectError.ok())
    {
        std::cerr
            << "Failed to connect to PacketForge server: "
            << connectError.message()
            << '\n';

        return 1;
    }

    std::cout
        << "PacketForge shutdown test client connected successfully\n";

    std::cout
        << "Connection state: "
        << (connection.isConnected() ? "CONNECTED" : "DISCONNECTED")
        << '\n';

    std::cout
        << "\nClient is now idle.\n"
        << "No packet will be created or sent.\n"
        << "The server should now be blocked in receive().\n"
        << "\nPress ENTER to disconnect the client after "
           "testing Ctrl+C on the server..."
        << std::endl;

    std::cin.get();

    connection.disconnect();

    std::cout
        << "PacketForge shutdown test client disconnected\n";

    return 0;
}
