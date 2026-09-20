#include "network/connection.hpp"
#include "network/socket.hpp"

#include <iostream>

int main()
{
    using namespace packetforge;

    network::Socket listeningSocket;
    network::Connection connection;

    std::cout
        << "GAP-003.2 Accept Failure Test\n"
        << "--------------------------------\n";

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

        return 1;
    }

    std::cout
        << "Accept failure detected successfully\n"
        << "Error code: "
        << static_cast<int>(acceptError.code())
        << '\n'
        << "Error message: "
        << acceptError.message()
        << '\n';

    if (acceptError.message() != "Listening socket is not open")
    {
        std::cerr
            << "FAIL: unexpected error message\n";

        return 1;
    }

    if (connection.isConnected())
    {
        std::cerr
            << "FAIL: connection state changed unexpectedly\n";

        return 1;
    }

    std::cout
        << "Connection state: "
        << (connection.isConnected()
                ? "CONNECTED"
                : "DISCONNECTED")
        << '\n';

    std::cout
        << "GAP-003.2 PASS\n";

    return 0;
}
