#include "network/connection.hpp"
#include "network/transport.hpp"

#include <arpa/inet.h>
#include <cerrno>
#include <csignal>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <utility>
#include <vector>

namespace
{

void closeSocket(int fd) noexcept
{
    if (fd >= 0)
    {
        ::close(fd);
    }
}

bool configureAbortiveClose(int fd)
{
    linger abortiveClose{};

    abortiveClose.l_onoff = 1;
    abortiveClose.l_linger = 0;

    return ::setsockopt(
               fd,
               SOL_SOCKET,
               SO_LINGER,
               &abortiveClose,
               sizeof(abortiveClose)
           ) == 0;
}

} // namespace

int main()
{
    using packetforge::common::ErrorCode;
    using packetforge::network::Connection;
    using packetforge::network::Transport;

    /*
     * ----------------------------------------------------------
     * GAP-033 — Deterministic Transport::send() Failure Test
     * ----------------------------------------------------------
     *
     * Purpose:
     *   Exercise the production Transport::send() error branch
     *   after the peer has forcibly reset the TCP connection.
     *
     * Expected production path:
     *
     *   peer TCP RST
     *        |
     *        v
     *   socket reports ECONNRESET
     *        |
     *        v
     *   Connection::connected_ remains true
     *        |
     *        v
     *   Transport::send()
     *        |
     *        v
     *   ::send() < 0
     *        |
     *        v
     *   ErrorCode::SocketError
     *   "Send failed"
     *
     * This test does not modify PacketForge production code or
     * the frozen V1 architecture.
     * ----------------------------------------------------------
     */

    std::cout
        << "GAP-033 Transport Send Failure Test\n";

    /*
     * Prevent SIGPIPE from terminating the test process when
     * the production Transport::send() writes to the reset peer.
     */
    if (std::signal(SIGPIPE, SIG_IGN) == SIG_ERR)
    {
        std::cerr
            << "FAIL: unable to ignore SIGPIPE\n";
        return 1;
    }

    /*
     * ----------------------------------------------------------
     * 033.1 — Create local listening socket
     * ----------------------------------------------------------
     */

    const int listeningFd =
        ::socket(AF_INET, SOCK_STREAM, 0);

    if (listeningFd < 0)
    {
        std::cerr
            << "FAIL: socket() failed: "
            << std::strerror(errno)
            << '\n';
        return 1;
    }

    int reuseAddress = 1;

    if (::setsockopt(
            listeningFd,
            SOL_SOCKET,
            SO_REUSEADDR,
            &reuseAddress,
            sizeof(reuseAddress)) < 0)
    {
        std::cerr
            << "FAIL: SO_REUSEADDR configuration failed: "
            << std::strerror(errno)
            << '\n';

        closeSocket(listeningFd);
        return 1;
    }

    sockaddr_in endpoint{};
    endpoint.sin_family = AF_INET;
    endpoint.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    endpoint.sin_port = htons(0);

    if (::bind(
            listeningFd,
            reinterpret_cast<sockaddr*>(&endpoint),
            sizeof(endpoint)) < 0)
    {
        std::cerr
            << "FAIL: bind() failed: "
            << std::strerror(errno)
            << '\n';

        closeSocket(listeningFd);
        return 1;
    }

    if (::listen(listeningFd, 1) < 0)
    {
        std::cerr
            << "FAIL: listen() failed: "
            << std::strerror(errno)
            << '\n';

        closeSocket(listeningFd);
        return 1;
    }

    socklen_t endpointLength =
        static_cast<socklen_t>(sizeof(endpoint));

    if (::getsockname(
            listeningFd,
            reinterpret_cast<sockaddr*>(&endpoint),
            &endpointLength) < 0)
    {
        std::cerr
            << "FAIL: getsockname() failed: "
            << std::strerror(errno)
            << '\n';

        closeSocket(listeningFd);
        return 1;
    }

    const std::uint16_t port =
        ntohs(endpoint.sin_port);

    std::cout
        << "Local listener: PASS\n"
        << "Test port: "
        << port
        << '\n';

    /*
     * ----------------------------------------------------------
     * 033.2 — Establish the PacketForge Connection
     * ----------------------------------------------------------
     */

    Connection connection;

    const auto connectError =
        connection.connect(
            "127.0.0.1",
            port
        );

    if (!connectError.ok())
    {
        std::cerr
            << "FAIL: PacketForge Connection failed: "
            << connectError.message()
            << '\n';

        closeSocket(listeningFd);
        return 1;
    }

    if (!connection.isConnected())
    {
        std::cerr
            << "FAIL: Connection state is not CONNECTED\n";

        connection.disconnect();
        closeSocket(listeningFd);
        return 1;
    }

    std::cout
        << "PacketForge connection: PASS\n"
        << "Connection state: CONNECTED\n";

    /*
     * Accept the peer corresponding to the PacketForge
     * Connection. This peer exists only to inject the TCP RST.
     */
    const int peerFd =
        ::accept(
            listeningFd,
            nullptr,
            nullptr
        );

    if (peerFd < 0)
    {
        std::cerr
            << "FAIL: accept() failed: "
            << std::strerror(errno)
            << '\n';

        connection.disconnect();
        closeSocket(listeningFd);
        return 1;
    }

    closeSocket(listeningFd);

    /*
     * ----------------------------------------------------------
     * 033.3 — Inject TCP reset from peer
     * ----------------------------------------------------------
     */

    if (!configureAbortiveClose(peerFd))
    {
        std::cerr
            << "FAIL: failed to configure abortive close: "
            << std::strerror(errno)
            << '\n';

        closeSocket(peerFd);
        connection.disconnect();
        return 1;
    }

    std::cout
        << "TCP reset configuration: PASS\n";

    closeSocket(peerFd);

    std::cout
        << "TCP reset injected: PASS\n";

    /*
     * ----------------------------------------------------------
     * 033.4 — Confirm that the connected socket sees the reset
     * ----------------------------------------------------------
     *
     * The raw recv() is deliberately performed on the same
     * socket owned by PacketForge. A reset peer is expected to
     * produce ECONNRESET here.
     *
     * Transport::receive() is not used because this test is
     * specifically validating Transport::send().
     */
    char probe = 0;

    const ssize_t received =
        ::recv(
            connection.socket().nativeHandle(),
            &probe,
            sizeof(probe),
            0
        );

    if (received >= 0)
    {
        std::cerr
            << "FAIL: expected recv() to report TCP reset, "
               "but it returned "
            << received
            << '\n';

        connection.disconnect();
        return 1;
    }

    if (errno != ECONNRESET)
    {
        std::cerr
            << "FAIL: expected ECONNRESET, got: "
            << std::strerror(errno)
            << '\n';

        connection.disconnect();
        return 1;
    }

    std::cout
        << "Peer reset observed by socket: PASS\n";

    /*
     * Transport::receive() maps recv()<0 to SocketError but
     * does not synchronize Connection::connected_ on that path.
     * Verify that the logical state is still CONNECTED so that
     * the following Transport::send() reaches ::send().
     */
    if (!connection.isConnected())
    {
        std::cerr
            << "FAIL: connection state unexpectedly changed to "
               "DISCONNECTED before send()\n";

        connection.disconnect();
        return 1;
    }

    std::cout
        << "Connection state before send(): CONNECTED\n";

    /*
     * ----------------------------------------------------------
     * 033.5 — Exercise production Transport::send()
     * ----------------------------------------------------------
     */

    Transport transport(
        std::move(connection)
    );

    const std::vector<std::uint8_t> data =
    {
        0x50,
        0x46,
        0x4B,
        0x54
    };

    const auto sendError =
        transport.send(data);

    /*
     * ----------------------------------------------------------
     * 033.6 — Validate the exact production failure result
     * ----------------------------------------------------------
     */

    if (sendError.ok())
    {
        std::cerr
            << "FAIL: Transport::send() unexpectedly succeeded\n";

        transport.connection().disconnect();
        return 1;
    }

    if (sendError.code() != ErrorCode::SocketError)
    {
        std::cerr
            << "FAIL: unexpected error code: "
            << static_cast<int>(sendError.code())
            << '\n';

        transport.connection().disconnect();
        return 1;
    }

    if (sendError.message() != "Send failed")
    {
        std::cerr
            << "FAIL: unexpected error message: "
            << sendError.message()
            << '\n';

        transport.connection().disconnect();
        return 1;
    }

    std::cout
        << "Transport::send() returned SocketError: PASS\n"
        << "Transport::send() message: "
        << sendError.message()
        << '\n';

    /*
     * ----------------------------------------------------------
     * 033.7 — Cleanup
     * ----------------------------------------------------------
     */

    transport.connection().disconnect();

    if (transport.isConnected())
    {
        std::cerr
            << "FAIL: connection remained CONNECTED after cleanup\n";
        return 1;
    }

    std::cout
        << "Connection cleanup: PASS\n"
        << "Connection state after cleanup: DISCONNECTED\n";

    std::cout
        << "GAP-033 transport send-failure injection: PASS\n";

    return 0;
}
