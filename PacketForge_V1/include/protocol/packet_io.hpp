#pragma once

#include "common/error.hpp"
#include "network/transport.hpp"
#include "protocol/opcode.hpp"
#include "protocol/packet.hpp"

namespace packetforge::protocol
{

class PacketIO
{
public:

    explicit PacketIO(
        network::Transport& transport
    );


    common::Error send(
        const Packet& packet
    );


    common::Error receive(
        Packet& packet,
        ProtocolError* protocolError = nullptr
    );


private:

    network::Transport& transport_;
};

} // namespace packetforge::protocol