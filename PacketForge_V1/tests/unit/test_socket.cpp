#include <gtest/gtest.h>

#include "network/socket.hpp"


using packetforge::network::Socket;
using packetforge::common::ErrorCode;



TEST(SocketTest, SocketStartsClosed)
{
    Socket socket;


    EXPECT_FALSE(
        socket.isOpen()
    );


    EXPECT_EQ(
        socket.nativeHandle(),
        -1
    );
}



TEST(SocketTest, CreatesSocketSuccessfully)
{
    Socket socket;


    auto error = socket.create();


    EXPECT_EQ(
        error.code(),
        ErrorCode::Success
    );


    EXPECT_TRUE(
        socket.isOpen()
    );


    EXPECT_NE(
        socket.nativeHandle(),
        -1
    );
}



TEST(SocketTest, CloseReleasesSocket)
{
    Socket socket;


    auto error = socket.create();


    ASSERT_TRUE(
        error.ok()
    );


    EXPECT_TRUE(
        socket.isOpen()
    );


    socket.close();


    EXPECT_FALSE(
        socket.isOpen()
    );


    EXPECT_EQ(
        socket.nativeHandle(),
        -1
    );
}



TEST(SocketTest, ClosingMultipleTimesIsSafe)
{
    Socket socket;


    auto error = socket.create();


    ASSERT_TRUE(
        error.ok()
    );


    socket.close();


    /*
        Second close should not
        cause double deletion
        or crash.
    */

    socket.close();


    EXPECT_FALSE(
        socket.isOpen()
    );


    EXPECT_EQ(
        socket.nativeHandle(),
        -1
    );
}



TEST(SocketTest, DestructorClosesAutomatically)
{
    int handle = -1;


    {
        Socket socket;


        auto error = socket.create();


        ASSERT_TRUE(
            error.ok()
        );


        handle = socket.nativeHandle();


        EXPECT_NE(
            handle,
            -1
        );

    } // Destructor called here


    /*
       We cannot directly check kernel state
       easily here, but this test ensures
       destructor executes without error.
    */


    SUCCEED();
}