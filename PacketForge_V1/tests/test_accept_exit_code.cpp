#include "network/connection.hpp"
#include "network/socket.hpp"

#include <iostream>

int main()
{
    using namespace packetforge;

    int exitCode = 0;

    network::Socket listeningSocket;
    network::Connection connection;

    std::cout
        << "GAP-003.9.5 Accept Failure Exit-Code Test\n"
        << "-------------------------------------------\n";

    std::cout
        << "Listening socket state: "
        << (listeningSocket.isOpen() ? "OPEN" : "CLOSED")
        << '\n';

    const auto acceptError =
        connection.accept(listeningSocket);

    if (acceptError.ok())
    {
        std::cerr
            << "FAIL: accept() unexpectedly succeeded\n";

        return 2;
    }

    std::cerr
        << "Accept failure detected: "
        << acceptError.message()
        << '\n';

    // Mirror the production server policy:
    // fatal accept failure => non-zero process exit.
    exitCode = 1;

    std::cout
        << "Expected fatal-error exit code: "
        << exitCode
        << '\n';

    std::cout
        << "Connection state after failure: "
        << (connection.isConnected()
                ? "CONNECTED"
                : "DISCONNECTED")
        << '\n';

    return exitCode;
}
