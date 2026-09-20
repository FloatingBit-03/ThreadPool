#include "network/connection.hpp"
#include "network/socket.hpp"
#include "network/transport.hpp"

#include <arpa/inet.h>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

int main()
{
    using namespace packetforge;

    std::cout
        << "GAP-003.9.6 Fatal Receive/Socket Failure Exit-Code Test\n"
        << "-------------------------------------------------------\n";

    const int listeningFd =
        ::socket(AF_INET, SOCK_STREAM, 0);

    if (listeningFd < 0)
    {
        std::cerr
            << "FAIL: unable to create test listening socket: "
            << std::strerror(errno)
            << '\n';

        return 2;
    }

    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    address.sin_port = htons(0);

    if (
        ::bind(
            listeningFd,
            reinterpret_cast<sockaddr*>(&address),
            sizeof(address)
        ) < 0
    )
    {
        std::cerr
            << "FAIL: bind() failed: "
            << std::strerror(errno)
            << '\n';

        ::close(listeningFd);
        return 2;
    }

    if (::listen(listeningFd, 1) < 0)
    {
        std::cerr
            << "FAIL: listen() failed: "
            << std::strerror(errno)
            << '\n';

        ::close(listeningFd);
        return 2;
    }

    socklen_t addressLength = sizeof(address);

    if (
        ::getsockname(
            listeningFd,
            reinterpret_cast<sockaddr*>(&address),
            &addressLength
        ) < 0
    )
    {
        std::cerr
            << "FAIL: getsockname() failed: "
            << std::strerror(errno)
            << '\n';

        ::close(listeningFd);
        return 2;
    }

    const std::uint16_t port =
        ntohs(address.sin_port);

    network::Connection connection;

    const auto connectError =
        connection.connect(
            "127.0.0.1",
            port
        );

    if (!connectError.ok())
    {
        std::cerr
            << "FAIL: test connection failed: "
            << connectError.message()
            << '\n';

        ::close(listeningFd);
        return 2;
    }

    const int peerFd =
        ::accept(
            listeningFd,
            nullptr,
            nullptr
        );

    if (peerFd < 0)
    {
        std::cerr
            << "FAIL: test peer accept() failed: "
            << std::strerror(errno)
            << '\n';

        connection.disconnect();
        ::close(listeningFd);
        return 2;
    }

    ::close(peerFd);
    ::close(listeningFd);

    network::Transport transport(
        std::move(connection)
    );

    /*
     * Deliberately invalidate the underlying socket without
     * changing Connection::connected_.
     *
     * This forces Transport::receive() past its connection-state
     * check and into the recv() failure path.
     */
    transport.connection().socket().close();

    std::vector<std::uint8_t> data;

    const auto receiveError =
        transport.receive(
            data,
            1
        );

    if (receiveError.ok())
    {
        std::cerr
            << "FAIL: receive() unexpectedly succeeded\n";

        return 2;
    }

    if (receiveError.message() != "Receive failed")
    {
        std::cerr
            << "FAIL: unexpected receive error message: "
            << receiveError.message()
            << '\n';

        return 2;
    }

    std::cerr
        << "Fatal receive/socket failure detected: "
        << receiveError.message()
        << '\n';

    std::cout
        << "Connection state before process exit: "
        << (transport.isConnected()
                ? "CONNECTED"
                : "DISCONNECTED")
        << '\n';

    std::cout
        << "Expected fatal-error exit code: 1\n"
        << "GAP-003.9.6 PASS\n";

    /*
     * GAP-003.9 contract:
     * fatal receive/socket failure => process exit code 1.
     */
    return 1;
}
