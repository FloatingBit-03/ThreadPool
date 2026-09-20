#include <arpa/inet.h>
#include <cerrno>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <sys/socket.h>
#include <unistd.h>

namespace
{
constexpr const char* SERVER_IP = "127.0.0.1";
constexpr std::uint16_t SERVER_PORT = 9090;

constexpr std::uint32_t MAGIC = 0x50464B54;
constexpr std::uint8_t VERSION = 1;
constexpr std::uint8_t FLAGS = 0;
constexpr std::uint16_t DISPATCH_FAILURE_TEST_OPCODE = 0xFFFF;
constexpr std::uint32_t SEQUENCE_ID = 1;
constexpr std::uint32_t PAYLOAD_LENGTH = 0;
constexpr std::size_t HEADER_SIZE = 16;

bool sendAll(
    int socketFd,
    const std::uint8_t* data,
    std::size_t size)
{
    std::size_t totalSent = 0;

    while (totalSent < size)
    {
        const ssize_t result =
            ::send(
                socketFd,
                data + totalSent,
                size - totalSent,
                0);

        if (result < 0)
        {
            std::cerr
                << "send() failed: "
                << std::strerror(errno)
                << '\n';

            return false;
        }

        if (result == 0)
        {
            std::cerr
                << "send() returned 0 before the complete packet was sent\n";

            return false;
        }

        totalSent += static_cast<std::size_t>(result);
    }

    return true;
}

void writeUint16(
    std::uint8_t* buffer,
    std::size_t offset,
    std::uint16_t value)
{
    const std::uint16_t networkValue = htons(value);

    std::memcpy(
        buffer + offset,
        &networkValue,
        sizeof(networkValue));
}

void writeUint32(
    std::uint8_t* buffer,
    std::size_t offset,
    std::uint32_t value)
{
    const std::uint32_t networkValue = htonl(value);

    std::memcpy(
        buffer + offset,
        &networkValue,
        sizeof(networkValue));
}
}

int main()
{
    std::cout
        << "PacketForge GAP-003.9.8 Dispatcher Failure Client\n"
        << "------------------------------------------------\n";

    const int socketFd =
        ::socket(
            AF_INET,
            SOCK_STREAM,
            0);

    if (socketFd < 0)
    {
        std::cerr
            << "Failed to create socket: "
            << std::strerror(errno)
            << '\n';

        return 1;
    }

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(SERVER_PORT);

    if (::inet_pton(
            AF_INET,
            SERVER_IP,
            &serverAddress.sin_addr) != 1)
    {
        std::cerr
            << "Failed to parse server address\n";

        ::close(socketFd);
        return 1;
    }

    std::cout
        << "Connecting to "
        << SERVER_IP
        << ':'
        << SERVER_PORT
        << "...\n";

    if (::connect(
            socketFd,
            reinterpret_cast<const sockaddr*>(&serverAddress),
            sizeof(serverAddress)) < 0)
    {
        std::cerr
            << "connect() failed: "
            << std::strerror(errno)
            << '\n';

        ::close(socketFd);
        return 1;
    }

    std::cout
        << "Connected successfully\n"
        << "Sending valid V1 packet with test opcode 0xFFFF...\n";

    std::uint8_t header[HEADER_SIZE]{};

    writeUint32(header, 0, MAGIC);
    header[4] = VERSION;
    header[5] = FLAGS;
    writeUint16(header, 6, DISPATCH_FAILURE_TEST_OPCODE);
    writeUint32(header, 8, SEQUENCE_ID);
    writeUint32(header, 12, PAYLOAD_LENGTH);

    if (!sendAll(
            socketFd,
            header,
            sizeof(header)))
    {
        ::close(socketFd);
        return 1;
    }

    std::cout
        << "Test packet sent successfully\n"
        << "Opcode: "
        << DISPATCH_FAILURE_TEST_OPCODE
        << " (0xFFFF)\n"
        << "Expected server action: dispatcher returns failure\n"
        << "Expected server exit code: 1\n";

    ::shutdown(
        socketFd,
        SHUT_WR);

    ::close(socketFd);

    return 0;
}
