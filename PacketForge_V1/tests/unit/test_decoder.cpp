#include <gtest/gtest.h>

#include "protocol/decoder.hpp"
#include "protocol/encoder.hpp"


using namespace packetforge::protocol;



TEST(
    DecoderTest,
    DecodesEmptyPacket
)
{

    Packet packet;
    Encoder encoder;

    auto data = 
         encoder.encode(packet);

    auto decoded =
        Decoder::decode(data);

    EXPECT_EQ(
        decoded.magicNumber(),
        Packet::MagicNumber
    );

    EXPECT_EQ(
        decoded.version(),
        1
    );

}



TEST(
    DecoderTest,
    DecodesPayload
)
{

    Packet packet;


    std::vector<uint8_t> payload =
    {
        'H','E','L','L','O'
    };


    packet.setPayload(payload);



    Encoder encoder;

    auto data = 
        encoder.encode(packet);


    auto decoded =
        Decoder::decode(data);



    EXPECT_EQ(
        decoded.payload(),
        payload
    );

}




TEST(
    DecoderTest,
    RejectsInvalidMagic
)
{

    std::vector<uint8_t> data =
    {
        0,0,0,0,
        1,0,
        0,1,
        0,0,0,1,
        0,0,0,0
    };


    EXPECT_THROW(
        Decoder::decode(data),
        std::runtime_error
    );

}




TEST(
    DecoderTest,
    RejectsInvalidPayloadLength
)
{

    Packet packet;


    Encoder encoder;

    auto data = 
        encoder.encode(packet);


    // corrupt payload length

    data[15]=10;



    EXPECT_THROW(
        Decoder::decode(data),
        std::runtime_error
    );

}