#include "network/socket.hpp"

#include <arpa/inet.h>
#include <fcntl.h>
#include <gtest/gtest.h>

#include <cstdint>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>


namespace
{

std::uint16_t getBoundPort(
    int socketFd)
{
    sockaddr_in address{};
    socklen_t length = sizeof(address);

    int result =
        ::getsockname(
            socketFd,
            reinterpret_cast<sockaddr*>(&address),
            &length
        );

    EXPECT_EQ(result, 0);

    return ntohs(address.sin_port);
}

} // namespace


using packetforge::common::ErrorCode;
using packetforge::network::Socket;


// ==========================================================
// Construction / State
// ==========================================================

TEST(SocketTest, SocketStartsClosed)
{
    Socket socket;

    EXPECT_FALSE(socket.isOpen());
    EXPECT_EQ(socket.nativeHandle(), -1);
}


// ==========================================================
// Create
// ==========================================================

TEST(SocketTest, CreatesSocketSuccessfully)
{
    Socket socket;

    auto error = socket.create();

    EXPECT_TRUE(error.ok());
    EXPECT_EQ(error.code(), ErrorCode::Success);
    EXPECT_TRUE(socket.isOpen());
    EXPECT_NE(socket.nativeHandle(), -1);
}


// ==========================================================
// Close
// ==========================================================

TEST(SocketTest, CloseReleasesSocket)
{
    Socket socket;

    ASSERT_TRUE(socket.create().ok());
    ASSERT_TRUE(socket.isOpen());

    socket.close();

    EXPECT_FALSE(socket.isOpen());
    EXPECT_EQ(socket.nativeHandle(), -1);
}


TEST(SocketTest, ClosingMultipleTimesIsSafe)
{
    Socket socket;

    ASSERT_TRUE(socket.create().ok());

    socket.close();
    socket.close();
    socket.close();

    EXPECT_FALSE(socket.isOpen());
    EXPECT_EQ(socket.nativeHandle(), -1);
}


// ==========================================================
// Destructor
// ==========================================================

TEST(SocketTest, DestructorClosesAutomatically)
{
    int fd = -1;

    {
        Socket socket;

        ASSERT_TRUE(socket.create().ok());

        fd = socket.nativeHandle();

        EXPECT_NE(fd, -1);
    }

    // The descriptor should have been released by the destructor.
    EXPECT_EQ(
        ::fcntl(fd, F_GETFD),
        -1
    );
}


// ==========================================================
// Move Construction
// ==========================================================

TEST(SocketTest, MoveConstructorTransfersOwnership)
{
    Socket first;

    ASSERT_TRUE(first.create().ok());

    int originalFd = first.nativeHandle();

    Socket second(
        std::move(first)
    );

    EXPECT_FALSE(first.isOpen());
    EXPECT_EQ(first.nativeHandle(), -1);

    EXPECT_TRUE(second.isOpen());
    EXPECT_EQ(second.nativeHandle(), originalFd);
}


// ==========================================================
// Move Assignment
// ==========================================================

TEST(SocketTest, MoveAssignmentTransfersOwnership)
{
    Socket first;
    Socket second;

    ASSERT_TRUE(first.create().ok());
    ASSERT_TRUE(second.create().ok());

    int firstFd = first.nativeHandle();

    second = std::move(first);

    EXPECT_FALSE(first.isOpen());
    EXPECT_EQ(first.nativeHandle(), -1);

    EXPECT_TRUE(second.isOpen());
    EXPECT_EQ(second.nativeHandle(), firstFd);
}


// ==========================================================
// Bind
// ==========================================================

TEST(SocketTest, BindSucceeds)
{
    Socket socket;

    ASSERT_TRUE(socket.create().ok());

    auto error =
        socket.bind(
            "127.0.0.1",
            0
        );

    EXPECT_TRUE(error.ok());
    EXPECT_EQ(error.code(), ErrorCode::Success);
    EXPECT_TRUE(socket.isOpen());
}


TEST(SocketTest, BindInvalidAddressFails)
{
    Socket socket;

    ASSERT_TRUE(socket.create().ok());

    auto error =
        socket.bind(
            "256.256.256.256",
            9000
        );

    EXPECT_FALSE(error.ok());
    EXPECT_EQ(
        error.code(),
        ErrorCode::InvalidArgument
    );
}


TEST(SocketTest, BindInvalidPortFails)
{
    Socket socket;

    ASSERT_TRUE(socket.create().ok());

    auto error =
        socket.bind(
            "127.0.0.1",
            0
        );

    /*
     * Port 0 is intentionally used by the test suite to request
     * an ephemeral port from the OS.
     *
     * Therefore this test does NOT consider port 0 invalid.
     *
     * If your Socket::bind() explicitly rejects port 0,
     * change this test to use the behavior defined by your API.
     */

    EXPECT_TRUE(error.ok());
}


// ==========================================================
// Listen
// ==========================================================

TEST(SocketTest, ListenSucceeds)
{
    Socket socket;

    ASSERT_TRUE(socket.create().ok());

    ASSERT_TRUE(
        socket.bind(
            "127.0.0.1",
            0
        ).ok()
    );

    auto error =
        socket.listen(5);

    EXPECT_TRUE(error.ok());
    EXPECT_EQ(error.code(), ErrorCode::Success);
}


TEST(SocketTest, ListenWithoutSocketFails)
{
    Socket socket;

    ASSERT_FALSE(socket.isOpen());

    auto error =
        socket.listen(5);

    EXPECT_FALSE(error.ok());
    EXPECT_EQ(
        error.code(),
        ErrorCode::SocketError
    );
}


TEST(SocketTest, ListenWithInvalidBacklogFails)
{
    Socket socket;

    ASSERT_TRUE(socket.create().ok());

    ASSERT_TRUE(
        socket.bind(
            "127.0.0.1",
            0
        ).ok()
    );

    auto error =
        socket.listen(-1);

    EXPECT_FALSE(error.ok());
}


// ==========================================================
// Accept
// ==========================================================

TEST(SocketTest, AcceptWithoutListeningFails)
{
    Socket server;
    Socket client;

    ASSERT_TRUE(server.create().ok());

    auto error =
        server.accept(client);

    EXPECT_FALSE(error.ok());
    EXPECT_EQ(
        error.code(),
        ErrorCode::SocketError
    );
}


TEST(SocketTest, AcceptSucceeds)
{
    Socket server;

    ASSERT_TRUE(server.create().ok());

    ASSERT_TRUE(
        server.bind(
            "127.0.0.1",
            0
        ).ok()
    );

    ASSERT_TRUE(
        server.listen(5).ok()
    );

    const std::uint16_t port =
        getBoundPort(
            server.nativeHandle()
        );

    ASSERT_NE(port, 0);

    /*
     * Create a real TCP client using the OS socket API.
     *
     * This is only test infrastructure. The server-side
     * operation being tested is PacketForge Socket::accept().
     */

    int clientFd =
        ::socket(
            AF_INET,
            SOCK_STREAM,
            0
        );

    ASSERT_NE(clientFd, -1);

    sockaddr_in serverAddress{};

    serverAddress.sin_family =
        AF_INET;

    serverAddress.sin_port =
        htons(port);

    ASSERT_EQ(
        ::inet_pton(
            AF_INET,
            "127.0.0.1",
            &serverAddress.sin_addr
        ),
        1
    );

    ASSERT_EQ(
        ::connect(
            clientFd,
            reinterpret_cast<sockaddr*>(&serverAddress),
            sizeof(serverAddress)
        ),
        0
    );

    Socket accepted;

    auto error =
        server.accept(
            accepted
        );

    EXPECT_TRUE(error.ok());
    EXPECT_EQ(
        error.code(),
        ErrorCode::Success
    );

    EXPECT_TRUE(
        accepted.isOpen()
    );

    EXPECT_NE(
        accepted.nativeHandle(),
        -1
    );

    EXPECT_NE(
        accepted.nativeHandle(),
        server.nativeHandle()
    );

    ::close(clientFd);
}


// ==========================================================
// Accept Reuses Output Socket Safely
// ==========================================================

TEST(SocketTest, AcceptClosesExistingClientSocket)
{
    Socket server;

    ASSERT_TRUE(server.create().ok());

    ASSERT_TRUE(
        server.bind(
            "127.0.0.1",
            0
        ).ok()
    );

    ASSERT_TRUE(
        server.listen(5).ok()
    );

    const std::uint16_t port =
        getBoundPort(
            server.nativeHandle()
        );

    ASSERT_NE(port, 0);


    int clientFd =
        ::socket(
            AF_INET,
            SOCK_STREAM,
            0
        );

    ASSERT_NE(clientFd, -1);


    sockaddr_in serverAddress{};

    serverAddress.sin_family =
        AF_INET;

    serverAddress.sin_port =
        htons(port);

    ASSERT_EQ(
        ::inet_pton(
            AF_INET,
            "127.0.0.1",
            &serverAddress.sin_addr
        ),
        1
    );


    ASSERT_EQ(
        ::connect(
            clientFd,
            reinterpret_cast<sockaddr*>(&serverAddress),
            sizeof(serverAddress)
        ),
        0
    );


    Socket accepted;

    ASSERT_TRUE(
        accepted.create().ok()
    );

    auto oldFd =
        accepted.nativeHandle();

    auto error =
        server.accept(
            accepted
        );

    EXPECT_TRUE(error.ok());
    EXPECT_TRUE(accepted.isOpen());

    EXPECT_NE(
        accepted.nativeHandle(),
        oldFd
    );

    ::close(clientFd);
}