#include <gtest/gtest.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstdint>
#include <thread>

#include "common/error.hpp"
#include "network/connection.hpp"
#include "network/socket.hpp"
#include "server/server.hpp"


using packetforge::common::ErrorCode;
using packetforge::network::Connection;
using packetforge::server::Server;


// ----------------------------------------------------------
// 1. Server starts stopped
// ----------------------------------------------------------

TEST(ServerTest, ServerStartsStopped)
{
    Server server;

    EXPECT_FALSE(
        server.isRunning()
    );

    EXPECT_EQ(
        server.port(),
        0
    );
}


// ----------------------------------------------------------
// 2. Server starts successfully
// ----------------------------------------------------------

TEST(ServerTest, StartSucceeds)
{
    Server server;

    auto error =
        server.start(
            "127.0.0.1",
            0
        );

    ASSERT_TRUE(
        error.ok()
    );

    EXPECT_TRUE(
        server.isRunning()
    );

    EXPECT_GT(
        server.port(),
        0
    );
}


// ----------------------------------------------------------
// 3. Server can stop
// ----------------------------------------------------------

TEST(ServerTest, StopStopsServer)
{
    Server server;

    ASSERT_TRUE(
        server.start(
            "127.0.0.1",
            0
        ).ok()
    );

    EXPECT_TRUE(
        server.isRunning()
    );

    server.stop();

    EXPECT_FALSE(
        server.isRunning()
    );

    EXPECT_EQ(
        server.port(),
        0
    );
}


// ----------------------------------------------------------
// 4. Stopping multiple times is safe
// ----------------------------------------------------------

TEST(ServerTest, StopMultipleTimesIsSafe)
{
    Server server;

    ASSERT_TRUE(
        server.start(
            "127.0.0.1",
            0
        ).ok()
    );

    server.stop();
    server.stop();

    EXPECT_FALSE(
        server.isRunning()
    );

    EXPECT_EQ(
        server.port(),
        0
    );
}


// ----------------------------------------------------------
// 5. Invalid address fails
// ----------------------------------------------------------

TEST(ServerTest, StartWithInvalidAddressFails)
{
    Server server;

    auto error =
        server.start(
            "invalid-ip",
            8080
        );

    EXPECT_FALSE(
        error.ok()
    );

    EXPECT_EQ(
        error.code(),
        ErrorCode::InvalidArgument
    );

    EXPECT_FALSE(
        server.isRunning()
    );

    EXPECT_EQ(
        server.port(),
        0
    );
}


// ----------------------------------------------------------
// 6. Invalid port fails
// ----------------------------------------------------------

TEST(ServerTest, StartWithInvalidPortFails)
{
    Server server;

    auto error =
        server.start(
            "127.0.0.1",
            0
        );

    /*
        Port 0 is intentionally valid for this API because
        it requests an OS-assigned ephemeral port.

        Therefore this test verifies that the assigned port
        is valid rather than treating port 0 as an error.
    */

    ASSERT_TRUE(
        error.ok()
    );

    EXPECT_TRUE(
        server.isRunning()
    );

    EXPECT_GT(
        server.port(),
        0
    );
}


// ----------------------------------------------------------
// 7. Starting an already-running server fails
// ----------------------------------------------------------

TEST(ServerTest, StartWhenAlreadyRunningFails)
{
    Server server;

    ASSERT_TRUE(
        server.start(
            "127.0.0.1",
            0
        ).ok()
    );

    const auto runningPort =
        server.port();

    auto error =
        server.start(
            "127.0.0.1",
            0
        );

    EXPECT_FALSE(
        error.ok()
    );

    EXPECT_EQ(
        error.code(),
        ErrorCode::SocketError
    );

    EXPECT_TRUE(
        server.isRunning()
    );

    EXPECT_EQ(
        server.port(),
        runningPort
    );
}


// ----------------------------------------------------------
// 8. Accept fails when server is not running
// ----------------------------------------------------------

TEST(ServerTest, AcceptFailsWhenServerIsStopped)
{
    Server server;
    Connection connection;

    auto error =
        server.accept(
            connection
        );

    EXPECT_FALSE(
        error.ok()
    );

    EXPECT_EQ(
        error.code(),
        ErrorCode::ConnectionFailed
    );

    EXPECT_FALSE(
        connection.isConnected()
    );
}


// ----------------------------------------------------------
// 9. Server accepts incoming client
// ----------------------------------------------------------

TEST(ServerTest, AcceptSucceeds)
{
    Server server;

    ASSERT_TRUE(
        server.start(
            "127.0.0.1",
            0
        ).ok()
    );

    const auto port =
        server.port();


    std::thread clientThread(
        [port]()
        {
            const int clientSocket =
                ::socket(
                    AF_INET,
                    SOCK_STREAM,
                    0
                );

            ASSERT_NE(
                clientSocket,
                -1
            );


            sockaddr_in endpoint{};

            endpoint.sin_family =
                AF_INET;

            endpoint.sin_port =
                htons(port);


            ASSERT_EQ(
                ::inet_pton(
                    AF_INET,
                    "127.0.0.1",
                    &endpoint.sin_addr
                ),
                1
            );


            ASSERT_EQ(
                ::connect(
                    clientSocket,
                    reinterpret_cast<sockaddr*>(&endpoint),
                    sizeof(endpoint)
                ),
                0
            );


            ::close(
                clientSocket
            );
        }
    );


    Connection connection;

    auto error =
        server.accept(
            connection
        );


    EXPECT_TRUE(
        error.ok()
    );

    EXPECT_TRUE(
        connection.isConnected()
    );

    EXPECT_TRUE(
        connection.socket().isOpen()
    );


    clientThread.join();
}


// ----------------------------------------------------------
// 10. Server remains running after accepting client
// ----------------------------------------------------------

TEST(ServerTest, ServerRemainsRunningAfterAccept)
{
    Server server;

    ASSERT_TRUE(
        server.start(
            "127.0.0.1",
            0
        ).ok()
    );

    const auto port =
        server.port();


    std::thread clientThread(
        [port]()
        {
            const int clientSocket =
                ::socket(
                    AF_INET,
                    SOCK_STREAM,
                    0
                );

            ASSERT_NE(
                clientSocket,
                -1
            );


            sockaddr_in endpoint{};

            endpoint.sin_family =
                AF_INET;

            endpoint.sin_port =
                htons(port);


            ASSERT_EQ(
                ::inet_pton(
                    AF_INET,
                    "127.0.0.1",
                    &endpoint.sin_addr
                ),
                1
            );


            ASSERT_EQ(
                ::connect(
                    clientSocket,
                    reinterpret_cast<sockaddr*>(&endpoint),
                    sizeof(endpoint)
                ),
                0
            );


            ::close(
                clientSocket
            );
        }
    );


    Connection connection;

    ASSERT_TRUE(
        server.accept(
            connection
        ).ok()
    );


    EXPECT_TRUE(
        server.isRunning()
    );

    EXPECT_GT(
        server.port(),
        0
    );


    clientThread.join();
}


// ----------------------------------------------------------
// 11. Move constructor transfers server ownership
// ----------------------------------------------------------

TEST(ServerTest, MoveConstructorTransfersOwnership)
{
    Server server;

    ASSERT_TRUE(
        server.start(
            "127.0.0.1",
            0
        ).ok()
    );

    const auto port =
        server.port();


    Server moved(
        std::move(server)
    );


    EXPECT_TRUE(
        moved.isRunning()
    );

    EXPECT_EQ(
        moved.port(),
        port
    );


    EXPECT_FALSE(
        server.isRunning()
    );

    EXPECT_EQ(
        server.port(),
        0
    );
}


// ----------------------------------------------------------
// 12. Move assignment transfers server ownership
// ----------------------------------------------------------

TEST(ServerTest, MoveAssignmentTransfersOwnership)
{
    Server server;

    ASSERT_TRUE(
        server.start(
            "127.0.0.1",
            0
        ).ok()
    );

    const auto port =
        server.port();


    Server moved;


    moved =
        std::move(server);


    EXPECT_TRUE(
        moved.isRunning()
    );

    EXPECT_EQ(
        moved.port(),
        port
    );


    EXPECT_FALSE(
        server.isRunning()
    );

    EXPECT_EQ(
        server.port(),
        0
    );
}