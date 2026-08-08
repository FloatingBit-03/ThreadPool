#include "protocol/decoder.hpp"

#include <stdexcept>


namespace packetforge::protocol
{


Packet Decoder::decode(
    const std::vector<std::uint8_t>& data
)
{

    if(data.size() < Packet::HEADER_SIZE)
    {
        throw std::runtime_error(
            "Invalid packet size"
        );
    }


    Packet packet;


    auto magic =
        readUint32(data,0);


    if(magic != Packet::MagicNumber)
    {
        throw std::runtime_error(
            "Invalid magic number"
        );
    }



    packet.setVersion(
        data[4]
    );


    packet.setFlags(
        data[5]
    );



    packet.setOpcode(
        readUint16(data,6)
    );



    packet.setSequenceId(
        readUint32(data,8)
    );



    auto length =
        readUint32(data,12);



    if(data.size() != Packet::HEADER_SIZE + length)
    {
        throw std::runtime_error(
            "Payload size mismatch"
        );
    }



    std::vector<uint8_t> payload;


    payload.insert(
        payload.end(),
        data.begin()+16,
        data.end()
    );


    packet.setPayload(payload);



    return packet;
}




std::uint32_t Decoder::readUint32(
    const std::vector<uint8_t>& data,
    std::size_t offset
)
{

    return
        (static_cast<uint32_t>(data[offset]) << 24) |
        (static_cast<uint32_t>(data[offset+1]) << 16) |
        (static_cast<uint32_t>(data[offset+2]) << 8) |
        static_cast<uint32_t>(data[offset+3]);

}




std::uint16_t Decoder::readUint16(
    const std::vector<uint8_t>& data,
    std::size_t offset
)
{

    return
        (static_cast<uint16_t>(data[offset]) << 8) |
        static_cast<uint16_t>(data[offset+1]);

}


}