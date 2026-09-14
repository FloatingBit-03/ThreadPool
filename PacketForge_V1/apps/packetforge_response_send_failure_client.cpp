#include <arpa/inet.h>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>

namespace
{
constexpr const char* SERVER_ADDRESS = "127.0.0.1";
constexpr std::uint16_t SERVER_PORT = 9090;

void closeSocket(int fd) noexcept
{
    if (fd >= 0)
    {
        ::close(fd);
    }
}
}

int main()
{
    std::cout
        << "PacketForge response-send-failure test client\n"
        << "Connecting to PacketForge server...\n";

    const int socketFd =
        ::socket(AF_INET, SOCK_STREAM, 0);

    if (socketFd < 0)
    {
        std::cerr
            << "Failed to create socket: "
            << std::strerror(errno)
            << '\n';
        return 1;
    }

    sockaddr_in endpoint{};
    endpoint.sin_family = AF_INET;
    endpoint.sin_port = htons(SERVER_PORT);

    if (::inet_pton(
            AF_INET,
            SERVER_ADDRESS,
            &endpoint.sin_addr) != 1)
    {
        std::cerr << "Invalid server address\n";
        closeSocket(socketFd);
        return 1;
    }

    if (::connect(
            socketFd,
            reinterpret_cast<sockaddr*>(&endpoint),
            sizeof(endpoint)) < 0)
    {
        std::cerr
            << "Failed to connect to PacketForge server: "
            << std::strerror(errno)
            << '\n';
        closeSocket(socketFd);
        return 1;
    }

    std::cout
        << "Fault-injection client connected successfully\n";

    // Valid PacketForge V1 HelloRequest:
    // Magic      = 0x50464B54 ("PFKT")
    // Version    = 1
    // Flags      = 0
    // Opcode     = 1 (HelloRequest)
    // Sequence   = 1
    // Payload    = 17 bytes ("Hello PacketForge")
    const std::vector<std::uint8_t> packet =
    {
        0x50, 0x46, 0x4B, 0x54,
        0x01,
        0x00,
        0x00, 0x01,
        0x00, 0x00, 0x00, 0x01,
        0x00, 0x00, 0x00, 0x11,
        'H', 'e', 'l', 'l', 'o', ' ',
        'P', 'a', 'c', 'k', 'e', 't', 'F', 'o', 'r', 'g', 'e'
    };

    const auto sent =
        ::send(
            socketFd,
            packet.data(),
            packet.size(),
            MSG_NOSIGNAL);

    if (sent != static_cast<ssize_t>(packet.size()))
    {
        std::cerr
            << "Failed to send HelloRequest: "
            << (sent < 0 ? std::strerror(errno) : "partial send")
            << '\n';
        closeSocket(socketFd);
        return 1;
    }

    std::cout
        << "Valid HelloRequest sent successfully\n";

    // Force an abortive close (TCP RST) so that the server's
    // subsequent response send is much more likely to fail.
    linger abortiveClose{};
    abortiveClose.l_onoff = 1;
    abortiveClose.l_linger = 0;

    if (::setsockopt(
            socketFd,
            SOL_SOCKET,
            SO_LINGER,
            &abortiveClose,
            sizeof(abortiveClose)) < 0)
    {
        std::cerr
            << "Failed to configure abortive close: "
            << std::strerror(errno)
            << '\n';
        closeSocket(socketFd);
        return 1;
    }

    std::cout
        << "Closing connection with TCP reset to trigger "
           "server response-send failure...\n";

    closeSocket(socketFd);

    std::cout
        << "Fault-injection client disconnected\n";

    return 0;
}
