#include "network/connection.hpp"

#include <cerrno>
#include <cstring>
#include <iostream>
#include <sys/socket.h>

namespace
{

constexpr const char* SERVER_ADDRESS = "127.0.0.1";
constexpr unsigned short SERVER_PORT = 9090;

} // namespace

int main()
{
    using packetforge::network::Connection;

    std::cout
        << "GAP-032 Receive-Side Failure Test\n";

    Connection connection;

    const auto connectError =
        connection.connect(
            SERVER_ADDRESS,
            SERVER_PORT
        );

    if (!connectError.ok())
    {
        std::cerr
            << "FAIL: Connection failed: "
            << connectError.message()
            << '\n';

        return 1;
    }

    if (!connection.isConnected())
    {
        std::cerr
            << "FAIL: Connection state is not CONNECTED\n";

        return 1;
    }

    std::cout
        << "Connection: PASS\n";

    // Force TCP RST on close. The server is expected to be
    // blocked in recv() and observe a socket-level receive error.
    linger resetOnClose{};
    resetOnClose.l_onoff = 1;
    resetOnClose.l_linger = 0;

    if (::setsockopt(
            connection.socket().nativeHandle(),
            SOL_SOCKET,
            SO_LINGER,
            &resetOnClose,
            sizeof(resetOnClose)) < 0)
    {
        std::cerr
            << "FAIL: setsockopt(SO_LINGER) failed: "
            << std::strerror(errno)
            << '\n';

        connection.disconnect();
        return 1;
    }

    std::cout
        << "TCP reset configuration: PASS\n";

    connection.disconnect();

    if (connection.isConnected())
    {
        std::cerr
            << "FAIL: Client remained CONNECTED after reset close\n";

        return 1;
    }

    std::cout
        << "TCP reset sent successfully\n"
        << "Client state: DISCONNECTED\n"
        << "GAP-032 receive-side failure injection: PASS\n";

    return 0;
}
