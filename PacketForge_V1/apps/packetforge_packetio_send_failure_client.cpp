#include "network/connection.hpp"
#include "network/transport.hpp"

#include "protocol/opcode.hpp"
#include "protocol/packet.hpp"
#include "protocol/packet_io.hpp"

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

bool createListener(
    int& listeningFd,
    std::uint16_t& port)
{
    listeningFd =
        ::socket(AF_INET, SOCK_STREAM, 0);

    if (listeningFd < 0)
    {
        return false;
    }

    int reuseAddress = 1;

    if (::setsockopt(
            listeningFd,
            SOL_SOCKET,
            SO_REUSEADDR,
            &reuseAddress,
            sizeof(reuseAddress)) < 0)
    {
        closeSocket(listeningFd);
        listeningFd = -1;
        return false;
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
        closeSocket(listeningFd);
        listeningFd = -1;
        return false;
    }

    if (::listen(listeningFd, 1) < 0)
    {
        closeSocket(listeningFd);
        listeningFd = -1;
        return false;
    }

    socklen_t endpointLength =
        static_cast<socklen_t>(sizeof(endpoint));

    if (::getsockname(
            listeningFd,
            reinterpret_cast<sockaddr*>(&endpoint),
            &endpointLength) < 0)
    {
        closeSocket(listeningFd);
        listeningFd = -1;
        return false;
    }

    port = ntohs(endpoint.sin_port);

    return true;
}

bool expectConnectionReset(
    packetforge::network::Connection& connection)
{
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

        return false;
    }

    if (errno != ECONNRESET)
    {
        std::cerr
            << "FAIL: expected ECONNRESET, got: "
            << std::strerror(errno)
            << '\n';

        return false;
    }

    return true;
}

} // namespace

int main()
{
    using packetforge::common::ErrorCode;
    using packetforge::network::Connection;
    using packetforge::network::Transport;
    using packetforge::protocol::Opcode;
    using packetforge::protocol::Packet;
    using packetforge::protocol::PacketIO;

    /*
     * ----------------------------------------------------------
     * GAP-033 — PacketIO::send() Failure Propagation Test
     * ----------------------------------------------------------
     *
     * Purpose:
     *   Validate that PacketIO::send() encodes a valid packet and
     *   propagates the production Transport::send() SocketError
     *   without converting it into a different error category.
     *
     * Production flow under test:
     *
     *   valid Packet
     *        |
     *        v
     *   PacketIO::send()
     *        |
     *        v
     *   Encoder::encode()
     *        |
     *        v
     *   Transport::send()
     *        |
     *        v
     *   underlying ::send() fails
     *        |
     *        v
     *   SocketError / "Send failed"
     *
     * No production source or frozen V1 architecture is changed.
     * ----------------------------------------------------------
     */

    std::cout
        << "GAP-033 PacketIO Send Failure Propagation Test\n";

    /*
     * Prevent SIGPIPE from terminating the test process.
     */
    if (std::signal(SIGPIPE, SIG_IGN) == SIG_ERR)
    {
        std::cerr
            << "FAIL: unable to ignore SIGPIPE\n";
        return 1;
    }

    /*
     * ----------------------------------------------------------
     * 033.1 — Create local peer
     * ----------------------------------------------------------
     */

    int listeningFd = -1;
    std::uint16_t port = 0;

    if (!createListener(listeningFd, port))
    {
        std::cerr
            << "FAIL: unable to create local TCP listener: "
            << std::strerror(errno)
            << '\n';
        return 1;
    }

    std::cout
        << "Local listener: PASS\n"
        << "Test port: "
        << port
        << '\n';

    /*
     * ----------------------------------------------------------
     * 033.2 — Establish PacketForge Connection
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
            << "FAIL: connection state is not CONNECTED\n";

        connection.disconnect();
        closeSocket(listeningFd);
        return 1;
    }

    std::cout
        << "PacketForge connection: PASS\n"
        << "Connection state: CONNECTED\n";

    /*
     * Accept the peer corresponding to the PacketForge
     * connection.
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
            << "FAIL: peer accept() failed: "
            << std::strerror(errno)
            << '\n';

        connection.disconnect();
        closeSocket(listeningFd);
        return 1;
    }

    closeSocket(listeningFd);
    listeningFd = -1;

    /*
     * ----------------------------------------------------------
     * 033.3 — Inject deterministic TCP reset
     * ----------------------------------------------------------
     */

    if (!configureAbortiveClose(peerFd))
    {
        std::cerr
            << "FAIL: failed to configure TCP abortive close: "
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
     * 033.4 — Confirm reset on PacketForge socket
     * ----------------------------------------------------------
     *
     * This is intentionally a raw recv() on the socket owned by
     * the PacketForge Connection. We do not call Transport::receive()
     * because this test is validating the send path.
     */
    if (!expectConnectionReset(connection))
    {
        connection.disconnect();
        return 1;
    }

    std::cout
        << "Peer reset observed by socket: PASS\n";

    /*
     * The current Connection implementation synchronizes
     * connected_ to false on recv() == 0. For recv() < 0 it
     * returns the socket error without clearing connected_.
     *
     * Therefore verify that PacketIO::send() can still reach
     * the production Transport::send() implementation.
     */
    if (!connection.isConnected())
    {
        std::cerr
            << "FAIL: connection unexpectedly became DISCONNECTED "
               "before PacketIO::send()\n";

        connection.disconnect();
        return 1;
    }

    std::cout
        << "Connection state before PacketIO::send(): CONNECTED\n";

    /*
     * ----------------------------------------------------------
     * 033.5 — Create valid PacketForge V1 packet
     * ----------------------------------------------------------
     */

    Packet packet;

    packet.setVersion(Packet::VERSION);
    packet.setFlags(0);
    packet.setOpcode(
        static_cast<std::uint16_t>(
            Opcode::HelloRequest
        )
    );
    packet.setSequenceId(1);

    packet.setPayload(
        {
            'H', 'e', 'l', 'l', 'o', ' ',
            'P', 'a', 'c', 'k', 'e', 't',
            'F', 'o', 'r', 'g', 'e'
        }
    );

    if (!packet.isValid())
    {
        std::cerr
            << "FAIL: valid HelloRequest packet failed validation\n";

        connection.disconnect();
        return 1;
    }

    std::cout
        << "Valid HelloRequest packet: PASS\n";

    /*
     * ----------------------------------------------------------
     * 033.6 — Exercise PacketIO::send()
     * ----------------------------------------------------------
     */

    Transport transport(
        std::move(connection)
    );

    PacketIO packetIO(transport);

    const auto sendError =
        packetIO.send(packet);

    /*
     * ----------------------------------------------------------
     * 033.7 — Validate propagation
     * ----------------------------------------------------------
     *
     * Expected:
     *
     *   PacketIO::send()
     *      -> Encoder::encode()
     *      -> Transport::send()
     *      -> SocketError
     *      -> "Send failed"
     *
     * PacketIO must not convert this transport error to
     * SerializationError or UnknownError.
     */

    if (sendError.ok())
    {
        std::cerr
            << "FAIL: PacketIO::send() unexpectedly succeeded\n";

        transport.connection().disconnect();
        return 1;
    }

    if (sendError.code() != ErrorCode::SocketError)
    {
        std::cerr
            << "FAIL: PacketIO::send() returned unexpected error code: "
            << static_cast<int>(sendError.code())
            << '\n';

        transport.connection().disconnect();
        return 1;
    }

    if (sendError.message() != "Send failed")
    {
        std::cerr
            << "FAIL: PacketIO::send() returned unexpected error "
               "message: "
            << sendError.message()
            << '\n';

        transport.connection().disconnect();
        return 1;
    }

    std::cout
        << "PacketIO::send() returned SocketError: PASS\n"
        << "PacketIO::send() message: "
        << sendError.message()
        << '\n';

    /*
     * ----------------------------------------------------------
     * 033.8 — Cleanup / final state
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
        << "GAP-033 PacketIO send-failure propagation: PASS\n";

    return 0;
}
