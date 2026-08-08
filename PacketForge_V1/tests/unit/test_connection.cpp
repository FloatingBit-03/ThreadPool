#include <gtest/gtest.h>


#include "network/connection.hpp"


using packetforge::network::Connection;
using packetforge::common::ErrorCode;



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