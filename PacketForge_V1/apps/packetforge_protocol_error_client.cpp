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
constexpr std::uint8_t INVALID_VERSION = 99;
constexpr std::uint8_t FLAGS = 0;
constexpr std::uint16_t OPCODE_HELLO_REQUEST = 1;
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

bool receiveAll(
    int socketFd,
    std::uint8_t* data,
    std::size_t size)
{
    std::size_t totalReceived = 0;

    while (totalReceived < size)
    {
        const ssize_t result =
            ::recv(
                socketFd,
                data + totalReceived,
                size - totalReceived,
                0);

        if (result < 0)
        {
            std::cerr
                << "recv() failed: "
                << std::strerror(errno)
                << '\n';

            return false;
        }

        if (result == 0)
        {
            std::cerr
                << "Connection closed before complete response was received\n";

            return false;
        }

        totalReceived +=
            static_cast<std::size_t>(result);
    }

    return true;
}

std::uint16_t readUint16(
    const std::uint8_t* buffer,
    std::size_t offset)
    {
        std::uint16_t value{};

        std::memcpy(
            &value,
            buffer + offset,
            sizeof(value));

        return ntohs(value);
    }

std::uint32_t readUint32(
    const std::uint8_t* buffer,
    std::size_t offset)
    {
        std::uint32_t value{};

        std::memcpy(
            &value,
            buffer + offset,
            sizeof(value));

        return ntohl(value);
    }

}

int main()
{
    std::cout
        << "PacketForge GAP-003.9.7 Protocol Error Client\n"
        << "----------------------------------------------\n";

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
        << "Sending deliberately malformed protocol header...\n";

    std::uint8_t header[HEADER_SIZE]{};

    // Header layout:
    // [0..3]   Magic       - uint32, network byte order
    // [4]      Version     - uint8
    // [5]      Flags       - uint8
    // [6..7]   Opcode      - uint16, network byte order
    // [8..11]  Sequence ID - uint32, network byte order
    // [12..15] Payload len - uint32, network byte order

    writeUint32(header, 0, MAGIC);

    // Deliberately invalid:
    // Packet::VERSION == 1
    // Decoder must raise ProtocolError::UnsupportedVersion.
    header[4] = INVALID_VERSION;

    header[5] = FLAGS;

    writeUint16(
        header,
        6,
        OPCODE_HELLO_REQUEST);

    writeUint32(
        header,
        8,
        SEQUENCE_ID);

    writeUint32(
        header,
        12,
        PAYLOAD_LENGTH);

    if (!sendAll(
            socketFd,
            header,
            sizeof(header)))
    {
        ::close(socketFd);
        return 1;
    }

    std::cout
        << "Malformed packet sent\n"
        << "Invalid protocol version: "
        << static_cast<unsigned int>(INVALID_VERSION)
        << '\n'
        << "Expected decoder result: UnsupportedVersion\n"
        << "Expected server ErrorCode: ProtocolError\n"
        << "Expected server exit code: 1\n";

    // The client only injects the malformed packet.
    // The server owns the protocol-error decision and exit status.
    std::cout
    << "Waiting for protocol error response...\n";

    /*
    * --------------------------------------------------------
    * Receive ErrorResponse header
    * --------------------------------------------------------
    */

    std::uint8_t responseHeader[HEADER_SIZE]{};

    if (!receiveAll(
            socketFd,
            responseHeader,
            sizeof(responseHeader)))
    {
        ::close(socketFd);
        return 1;
    }

    const auto responseMagic =
        readUint32(
            responseHeader,
            0);

    const auto responseVersion =
        responseHeader[4];

    const auto responseFlags =
        responseHeader[5];

    const auto responseOpcode =
        readUint16(
            responseHeader,
            6);

    const auto responseSequenceId =
        readUint32(
            responseHeader,
            8);

    const auto responsePayloadLength =
        readUint32(
            responseHeader,
            12);

    std::cout
        << "\nProtocol error response received\n"
        << "  Magic: 0x"
        << std::hex
        << responseMagic
        << std::dec
        << '\n'
        << "  Version: "
        << static_cast<int>(responseVersion)
        << '\n'
        << "  Flags: "
        << static_cast<int>(responseFlags)
        << '\n'
        << "  Opcode: "
        << responseOpcode
        << '\n'
        << "  Sequence ID: "
        << responseSequenceId
        << '\n'
        << "  Payload length: "
        << responsePayloadLength
        << '\n';

    if (responsePayloadLength != 1)
    {
        std::cerr
            << "Unexpected protocol error payload length\n";

        ::close(socketFd);
        return 1;
    }

    std::uint8_t responseErrorCode{};

    if (!receiveAll(
            socketFd,
            &responseErrorCode,
            sizeof(responseErrorCode)))
    {
        ::close(socketFd);
        return 1;
    }

    std::cout
        << "  Protocol error code: "
        << static_cast<int>(responseErrorCode)
        << '\n';

    constexpr std::uint16_t EXPECTED_ERROR_RESPONSE_OPCODE = 100;
    constexpr std::uint8_t EXPECTED_UNSUPPORTED_VERSION_ERROR = 3;

    const bool passed =
        responseMagic == MAGIC &&
        responseVersion == 1 &&
        responseOpcode == EXPECTED_ERROR_RESPONSE_OPCODE &&
        responseSequenceId == 0 &&
        responsePayloadLength == 1 &&
        responseErrorCode == EXPECTED_UNSUPPORTED_VERSION_ERROR;

    if (passed)
    {
        std::cout
            << "\nGAP-018 unsupported-version test: PASS\n";
    }
    else
    {
        std::cerr
            << "\nGAP-018 unsupported-version test: FAIL\n";

        ::close(socketFd);
        return 1;
    }

    ::close(socketFd);

    return 0;

}
