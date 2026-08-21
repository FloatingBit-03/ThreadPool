PacketIO Module — Design Draft

Location

include/protocol/packet_io.hpp
src/protocol/packet_io.cpp
Purpose

PacketIO is the protocol-level I/O layer between Transport and Client/Server.

Client / Server
      │
      ▼
   PacketIO
      │
      ▼
  Transport
      │
      ▼
 Connection / Socket
      │
      ▼
     TCP
Responsibilities

PacketIO should handle:

Send Packet
Take a protocol::Packet
Use Encoder to serialize it
Send serialized bytes through Transport
Receive Packet
Read the fixed 16-byte header through Transport
Determine payload length
Read the payload
Reconstruct the complete packet bytes
Use Decoder to create a Packet
Protocol framing
Understand where the packet starts/ends
Handle header + payload boundaries
Error translation
Convert encoding/decoding failures into common::Error
Propagate transport errors
What PacketIO should NOT do

It should not:

create sockets
bind/listen
accept connections
establish TCP connections
manage client/server lifecycle
contain application/business logic
directly call send() / recv()

Those responsibilities already belong to the lower layers.

Proposed API
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
        Packet& packet
    );


private:


    network::Transport& transport_;
};
Result

Then both sides become simple:

Client
  │
  └── PacketIO.send(packet)
              │
              ▼
          Transport

and:

Server
  │
  └── PacketIO.receive(packet)
              │
              ▼
          Transport

So the main architectural improvement is:
Move packet serialization/framing/deserialization out of Client and Server and centralize it in PacketIO.
This prevents the client and server from implementing the same packet-I/O logic independently.