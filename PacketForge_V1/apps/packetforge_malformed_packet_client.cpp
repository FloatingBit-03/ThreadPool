#include "network/connection.hpp"
#include "network/transport.hpp"

#include "protocol/decoder.hpp"
#include "protocol/opcode.hpp"
#include "protocol/packet.hpp"
#include "protocol/packet_io.hpp"

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

void closeSocket(int fd) noexcept
{
    if (fd >= 0)
    {
        ::close(fd);
    }
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

bool expectProtocolDecodeError(
    const std::vector<std::uint8_t>& data,
    packetforge::protocol::ProtocolError expectedError,
    const char* expectedMessage)
{
    try
    {
        (void)packetforge::protocol::Decoder::decode(data);

        std::cerr
            << "FAIL: Decoder unexpectedly accepted malformed packet\n";

        return false;
    }
    catch (const packetforge::protocol::ProtocolDecodeError& error)
    {
        if (error.error() != expectedError)
        {
            std::cerr
                << "FAIL: unexpected decoder error code: "
                << static_cast<int>(error.error())
                << '\n';

            return false;
        }

        if (std::strcmp(error.what(), expectedMessage) != 0)
        {
            std::cerr
                << "FAIL: unexpected decoder error message: "
                << error.what()
                << '\n';

            return false;
        }

        return true;
    }
    catch (const std::exception& error)
    {
        std::cerr
            << "FAIL: unexpected exception type: "
            << error.what()
            << '\n';

        return false;
    }
}

bool sendAll(
    int socketFd,
    const std::vector<std::uint8_t>& data)
{
    std::size_t totalSent = 0;

    while (totalSent < data.size())
    {
        const ssize_t result =
            ::send(
                socketFd,
                data.data() + totalSent,
                data.size() - totalSent,
                MSG_NOSIGNAL
            );

        if (result < 0)
        {
            std::cerr
                << "FAIL: send() failed: "
                << std::strerror(errno)
                << '\n';

            return false;
        }

        if (result == 0)
        {
            std::cerr
                << "FAIL: send() returned zero\n";

            return false;
        }

        totalSent +=
            static_cast<std::size_t>(result);
    }

    return true;
}

std::vector<std::uint8_t> makeInvalidMagicPacket()
{
    const std::uint16_t opcode =
        htons(
            static_cast<std::uint16_t>(
                packetforge::protocol::Opcode::HelloRequest
            )
        );

    const std::uint32_t sequenceId =
        htonl(1);

    const std::uint32_t payloadLength =
        htonl(0);

    std::vector<std::uint8_t> packet(
        packetforge::protocol::Packet::HEADER_SIZE,
        0
    );

    const std::uint32_t invalidMagic =
        htonl(0xDEADBEEFu);

    std::memcpy(
        packet.data(),
        &invalidMagic,
        sizeof(invalidMagic)
    );

    packet[4] = packetforge::protocol::Packet::VERSION;
    packet[5] = 0;

    std::memcpy(
        packet.data() + 6,
        &opcode,
        sizeof(opcode)
    );

    std::memcpy(
        packet.data() + 8,
        &sequenceId,
        sizeof(sequenceId)
    );

    std::memcpy(
        packet.data() + 12,
        &payloadLength,
        sizeof(payloadLength)
    );

    return packet;
}

std::vector<std::uint8_t> makePayloadMismatchPacket()
{
    const std::uint32_t magic =
        htonl(packetforge::protocol::Packet::MagicNumber);

    const std::uint16_t opcode =
        htons(
            static_cast<std::uint16_t>(
                packetforge::protocol::Opcode::HelloRequest
            )
        );

    const std::uint32_t sequenceId =
        htonl(1);

    /*
     * Header declares 4 payload bytes, but the complete buffer
     * contains only the 16-byte header. Decoder must reject it.
     */
    const std::uint32_t payloadLength =
        htonl(4);

    std::vector<std::uint8_t> packet(
        packetforge::protocol::Packet::HEADER_SIZE,
        0
    );

    std::memcpy(
        packet.data(),
        &magic,
        sizeof(magic)
    );

    packet[4] = packetforge::protocol::Packet::VERSION;
    packet[5] = 0;

    std::memcpy(
        packet.data() + 6,
        &opcode,
        sizeof(opcode)
    );

    std::memcpy(
        packet.data() + 8,
        &sequenceId,
        sizeof(sequenceId)
    );

    std::memcpy(
        packet.data() + 12,
        &payloadLength,
        sizeof(payloadLength)
    );

    return packet;
}

bool testPacketIODecodeFailurePropagation(
    const std::vector<std::uint8_t>& malformedPacket)
{
    int listeningFd = -1;
    std::uint16_t port = 0;

    if (!createListener(listeningFd, port))
    {
        std::cerr
            << "FAIL: local listener creation failed: "
            << std::strerror(errno)
            << '\n';

        return false;
    }

    packetforge::network::Connection connection;

    const auto connectError =
        connection.connect(
            "127.0.0.1",
            port
        );

    if (!connectError.ok())
    {
        std::cerr
            << "FAIL: PacketForge connection failed: "
            << connectError.message()
            << '\n';

        closeSocket(listeningFd);
        return false;
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
            << "FAIL: peer accept() failed: "
            << std::strerror(errno)
            << '\n';

        connection.disconnect();
        closeSocket(listeningFd);
        return false;
    }

    closeSocket(listeningFd);

    if (!sendAll(peerFd, malformedPacket))
    {
        closeSocket(peerFd);
        connection.disconnect();
        return false;
    }

    closeSocket(peerFd);

    packetforge::network::Transport transport(
        std::move(connection)
    );

    packetforge::protocol::PacketIO packetIO(
        transport
    );

    packetforge::protocol::Packet packet;

    packetforge::protocol::ProtocolError protocolError =
        packetforge::protocol::ProtocolError::InvalidPacket;

    const auto receiveError =
        packetIO.receive(
            packet,
            &protocolError
        );

    if (receiveError.ok())
    {
        std::cerr
            << "FAIL: PacketIO::receive() unexpectedly succeeded\n";

        transport.connection().disconnect();
        return false;
    }

    if (receiveError.code() !=
        packetforge::common::ErrorCode::ProtocolError)
    {
        std::cerr
            << "FAIL: PacketIO::receive() returned unexpected "
               "error code: "
            << static_cast<int>(receiveError.code())
            << '\n';

        transport.connection().disconnect();
        return false;
    }

    if (protocolError !=
        packetforge::protocol::ProtocolError::InvalidPacket)
    {
        std::cerr
            << "FAIL: PacketIO protocol error value is unexpected: "
            << static_cast<int>(protocolError)
            << '\n';

        transport.connection().disconnect();
        return false;
    }

    transport.connection().disconnect();

    return true;
}

} // namespace

int main()
{
    using packetforge::protocol::Packet;
    using packetforge::protocol::ProtocolError;

    /*
     * ----------------------------------------------------------
     * GAP-034 — Malformed Packet Handling
     * ----------------------------------------------------------
     *
     * This test validates malformed-frame detection at the
     * protocol decoder boundary and verifies PacketIO propagation
     * for a malformed packet that reaches the decoder.
     *
     * Cases:
     *
     *   034.1  Truncated header       -> InvalidPacket
     *   034.2  Invalid magic         -> InvalidPacket
     *   034.3  Payload size mismatch -> InvalidPayload
     *   034.4  PacketIO propagation  -> ProtocolError
     *
     * Unsupported protocol version is intentionally not repeated
     * here because it is already covered by GAP-024.
     *
     * Truncated network data is also distinct from decoder
     * malformed-packet handling: when PacketIO cannot assemble
     * the complete declared frame, Transport reports the transport
     * condition first.
     * ----------------------------------------------------------
     */

    std::cout
        << "GAP-034 Malformed Packet Handling Test\n";

    /*
     * ----------------------------------------------------------
     * 034.1 — Truncated header
     * ----------------------------------------------------------
     */

    const std::vector<std::uint8_t> truncatedHeader =
    {
        0x50,
        0x46,
        0x4B,
        0x54,
        0x01,
        0x00,
        0x00,
        0x01
    };

    if (!expectProtocolDecodeError(
            truncatedHeader,
            ProtocolError::InvalidPacket,
            "Invalid packet size"))
    {
        return 1;
    }

    std::cout
        << "Truncated header rejected: PASS\n";

    /*
     * ----------------------------------------------------------
     * 034.2 — Invalid magic
     * ----------------------------------------------------------
     */

    const auto invalidMagicPacket =
        makeInvalidMagicPacket();

    if (!expectProtocolDecodeError(
            invalidMagicPacket,
            ProtocolError::InvalidPacket,
            "Invalid magic number"))
    {
        return 1;
    }

    std::cout
        << "Invalid magic rejected: PASS\n";

    /*
     * ----------------------------------------------------------
     * 034.3 — Payload size mismatch
     * ----------------------------------------------------------
     */

    const auto payloadMismatchPacket =
        makePayloadMismatchPacket();

    if (!expectProtocolDecodeError(
            payloadMismatchPacket,
            ProtocolError::InvalidPayload,
            "Payload size mismatch"))
    {
        return 1;
    }

    std::cout
        << "Payload size mismatch rejected: PASS\n";

    /*
     * ----------------------------------------------------------
     * 034.4 — PacketIO propagation
     * ----------------------------------------------------------
     *
     * Send a complete malformed packet over a real TCP connection
     * so PacketIO::receive() reaches Decoder::decode().
     */
    if (!testPacketIODecodeFailurePropagation(
            invalidMagicPacket))
    {
        return 1;
    }

    std::cout
        << "PacketIO malformed-packet propagation: PASS\n";

    std::cout
        << "GAP-034 malformed packet handling: PASS\n";

    return 0;
}
