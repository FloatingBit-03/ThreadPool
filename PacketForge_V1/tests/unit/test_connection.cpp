#include <gtest/gtest.h>

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <atomic>
#include <thread>
#include <utility>

#include "network/connection.hpp"
#include "network/socket.hpp"


using packetforge::common::ErrorCode;
using packetforge::network::Connection;
using packetforge::network::Socket;


// ----------------------------------------------------------
// Basic State
// ----------------------------------------------------------

TEST(ConnectionTest, ConnectionStartsDisconnected)
{
    Connection connection;

    EXPECT_FALSE(
        connection.isConnected()
    );

    EXPECT_FALSE(
        connection.socket().isOpen()
    );
}


// ----------------------------------------------------------
// Open
// ----------------------------------------------------------

TEST(ConnectionTest, OpenCreatesSocketSuccessfully)
{
    Connection connection;

    auto error =
        connection.open();

    EXPECT_EQ(
        error.code(),
        ErrorCode::Success
    );

    EXPECT_TRUE(
        connection.socket().isOpen()
    );

    EXPECT_FALSE(
        connection.isConnected()
    );
}


// ----------------------------------------------------------
// Disconnect
// ----------------------------------------------------------

TEST(ConnectionTest, DisconnectClosesSocket)
{
    Connection connection;

    auto error =
        connection.open();

    ASSERT_TRUE(
        error.ok()
    );

    connection.disconnect();

    EXPECT_FALSE(
        connection.isConnected()
    );

    EXPECT_FALSE(
        connection.socket().isOpen()
    );
}


TEST(ConnectionTest, DisconnectMultipleTimesIsSafe)
{
    Connection connection;

    auto error =
        connection.open();

    ASSERT_TRUE(
        error.ok()
    );

    connection.disconnect();

    /*
        Second disconnect should not
        crash or double close.
    */

    connection.disconnect();

    EXPECT_FALSE(
        connection.isConnected()
    );

    EXPECT_FALSE(
        connection.socket().isOpen()
    );
}


// ----------------------------------------------------------
// Client-side connect()
// ----------------------------------------------------------

TEST(ConnectionTest, InvalidIPAddressReturnsError)
{
    Connection connection;

    auto error =
        connection.connect(
            "invalid-ip",
            8080
        );

    EXPECT_EQ(
        error.code(),
        ErrorCode::InvalidArgument
    );

    EXPECT_FALSE(
        connection.isConnected()
    );
}


TEST(ConnectionTest, InvalidPortReturnsError)
{
    Connection connection;

    auto error =
        connection.connect(
            "127.0.0.1",
            0
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
// Move Semantics
// ----------------------------------------------------------

TEST(ConnectionTest, MoveConstructorTransfersOwnership)
{
    Connection connection;

    auto error =
        connection.open();

    ASSERT_TRUE(
        error.ok()
    );

    Connection moved(
        std::move(connection)
    );

    EXPECT_TRUE(
        moved.socket().isOpen()
    );

    EXPECT_FALSE(
        connection.socket().isOpen()
    );
}


TEST(ConnectionTest, MoveAssignmentTransfersOwnership)
{
    Connection connection;

    auto error =
        connection.open();

    ASSERT_TRUE(
        error.ok()
    );

    Connection moved;

    moved =
        std::move(connection);

    EXPECT_TRUE(
        moved.socket().isOpen()
    );

    EXPECT_FALSE(
        connection.socket().isOpen()
    );
}


// ----------------------------------------------------------
// Server-side accept()
// ----------------------------------------------------------

TEST(ConnectionTest, AcceptsIncomingConnection)
{
    Socket serverSocket;

    /*
        Create listening socket.
    */

    ASSERT_TRUE(
        serverSocket.create().ok()
    );

    /*
        Bind to localhost using port 0.

        Port 0 asks the OS to automatically
        select an available ephemeral port.
    */

    ASSERT_TRUE(
        serverSocket.bind(
            "127.0.0.1",
            0
        ).ok()
    );

    /*
        Start listening.
    */

    ASSERT_TRUE(
        serverSocket.listen(1).ok()
    );

    /*
        Discover the port selected by the OS.
    */

    sockaddr_in address{};

    socklen_t addressLength =
        sizeof(address);

    ASSERT_EQ(
        ::getsockname(
            serverSocket.nativeHandle(),
            reinterpret_cast<sockaddr*>(&address),
            &addressLength
        ),
        0
    );

    const auto port =
        ntohs(address.sin_port);


    /*
        Client-side result.

        The worker thread cannot directly use
        ASSERT_* safely, so we store the result
        and check it from the main test thread.
    */

    std::atomic<bool> clientConnected{
        false
    };


    /*
        Start a temporary client.

        This connect() call will unblock
        Connection::accept().
    */

    std::thread clientThread(
        [port, &clientConnected]()
        {
            Socket clientSocket;

            auto createError =
                clientSocket.create();

            if (!createError.ok())
            {
                return;
            }


            sockaddr_in endpoint{};

            endpoint.sin_family =
                AF_INET;

            endpoint.sin_port =
                htons(port);


            if (::inet_pton(
                    AF_INET,
                    "127.0.0.1",
                    &endpoint.sin_addr
                ) != 1)
            {
                return;
            }


            const int result =
                ::connect(
                    clientSocket.nativeHandle(),
                    reinterpret_cast<sockaddr*>(&endpoint),
                    sizeof(endpoint)
                );


            if (result == 0)
            {
                clientConnected = true;
            }
        }
    );


    /*
        Server accepts the incoming connection.
    */

    Connection connection;

    auto error =
        connection.accept(
            serverSocket
        );


    /*
        Wait for the client thread to finish.
    */

    clientThread.join();


    /*
        Verify the complete operation.
    */

    EXPECT_TRUE(
        clientConnected.load()
    );

    EXPECT_TRUE(
        error.ok()
    );

    EXPECT_EQ(
        error.code(),
        ErrorCode::Success
    );

    EXPECT_TRUE(
        connection.isConnected()
    );

    EXPECT_TRUE(
        connection.socket().isOpen()
    );
}


TEST(ConnectionTest, AcceptFailsWithoutListeningSocket)
{
    Socket serverSocket;

    /*
        Socket exists as an object but has
        no native socket underneath it.
    */

    Connection connection;

    auto error =
        connection.accept(
            serverSocket
        );

    EXPECT_FALSE(
        error.ok()
    );

    EXPECT_EQ(
        error.code(),
        ErrorCode::SocketError
    );

    EXPECT_FALSE(
        connection.isConnected()
    );

    EXPECT_FALSE(
        connection.socket().isOpen()
    );
}


// ----------------------------------------------------------
// Destructor
// ----------------------------------------------------------

TEST(ConnectionTest, DestructorReleasesResources)
{
    {
        Connection connection;

        auto error =
            connection.open();

        ASSERT_TRUE(
            error.ok()
        );

        EXPECT_TRUE(
            connection.socket().isOpen()
        );
    }

    SUCCEED();
} 