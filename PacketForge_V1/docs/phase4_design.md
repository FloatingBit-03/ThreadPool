PacketForge — Phase 4 Design Document

1. Overview
Phase 4 introduces the Client layer of PacketForge.
The objective of Phase 4 is to provide a high-level API that allows an application to establish a network connection and exchange PacketForge protocol packets without directly managing sockets, connections, transports, encoding, or decoding.
Phase 4 builds on the infrastructure implemented in previous phases:

Phase 1 — Common utilities and Endian support
Phase 2 — Network abstraction
Phase 3 — Protocol packet representation and serialization
Phase 4 — Client abstraction

The primary module introduced by Phase 4 is:Client

The Client layer integrates the existing:
Transport
Connection
Socket
Packet
Encoder
Decoder

The resulting abstraction is:

Application
     |
     v
+-----------+
|  Client   |
+-----------+
     |
     v
+-----------+
| Transport |
+-----------+
     |
     v
+------------+
| Connection |
+------------+
     |
     v
+-----------+
|   Socket  |
+-----------+

Packet communication follows:

             Sending

Application
     |
     | Packet
     v
  Client
     |
     | encode
     v
  Encoder
     |
     | byte vector
     v
 Transport
     |
     v
 Connection
     |
     v
  Socket
     |
     v
 Network

Receiving follows the reverse direction:

Network
   |
   v
 Socket
   |
   v
Connection
   |
   v
Transport
   |
   | byte vector
   v
 Decoder
   |
   | Packet
   v
 Client
   |
   v
Application

2. Phase 4 Objectives
The primary objectives of Phase 4 are:

Introduce a high-level Client abstraction.
Hide lower-level network implementation details from users.
Integrate Client with the existing Transport.
Provide connection management through the Client API.
Provide packet-based send functionality.
Provide packet-based receive functionality.
Reuse the existing Encoder.
Reuse the existing Decoder.
Preserve the ownership model established by the network layer.
Maintain RAII-based resource management.
Support move semantics.
Prevent accidental copying of network resources.
Provide unit tests for Client behavior.
Integrate Client tests into the project build system.

3. Client Module
3.1 Purpose
The Client represents the application-facing network client.
It provides a simplified interface for:

connecting to a remote endpoint,
disconnecting,
checking connection state,
sending protocol packets,
receiving protocol packets.

The Client should not expose socket-level operations to the application.
The application therefore interacts with:

Client client;

client.connect(...);
client.send(packet);
client.receive(packet);
client.disconnect();

rather than directly interacting with:

Socket
Connection
Transport
Encoder
Decoder

4. Client API
The final Client interface is deliberately exposes Packet rather than raw byte buffers.
This ensures that packet serialization remains an internal implementation concern of the Client.

5. Client Ownership Model
The Client owns a Transport object: network::Transport transport_;

The ownership hierarchy is therefore:

Client
  |
  owns
  v
Transport
  |
  owns
  v
Connection
  |
  owns
  v
Socket

This follows the RAII model already established in the network layer.
The Client does not directly own a socket.

6. Construction and Destruction
6.1 Constructor
A newly constructed Client starts disconnected.

Client client;

client.isConnected(); // false

No network connection is established by the constructor.
Connection establishment is explicitly controlled by connect().

6.2 Destructor
The Client destructor releases its owned resources through the underlying RAII objects.
The Client therefore does not require explicit socket cleanup from the application.

7. Copy and Move Semantics
7.1 Copying
Client copying is disabled:

Client(const Client&) = delete;

Client& operator=(
    const Client&
) = delete;

This prevents accidental duplication of network ownership.

7.2 Move Construction

Client objects are movable: Client(Client&& other) noexcept;
Move construction transfers the underlying transport state.

7.3 Move Assignment

Move assignment is also supported:

Client& operator=(
    Client&& other
) noexcept;

This allows ownership of a Client connection to be transferred safely.

8. Connection Management
8.1 Connect
The Client exposes:

common::Error connect(
    const std::string& host,
    std::uint16_t port);

The Client delegates connection establishment to the underlying Connection.

The logical flow is:

Client::connect()
       |
       v
Transport::connection()
       |
       v
Connection::connect()
       |
       v
Socket

The Client does not duplicate socket connection logic.

9. Disconnect
The Client exposes:

void disconnect() noexcept;

Disconnecting delegates to the underlying connection.

Calling disconnect() when the Client is already disconnected is safe.
This follows the behavior established by the lower network layers.

10. Connection State
The Client exposes: bool isConnected() const noexcept;

The Client delegates this state query to:Transport::isConnected()

which ultimately reflects the state maintained by Connection.
The Client therefore has a single authoritative connection state.

11. Packet Sending

The Client exposes:
common::Error send(
    const protocol::Packet& packet);

The application provides an in-memory Packet.

The Client performs:
Packet
  |
  v
Encoder
  |
  v
std::vector<uint8_t>
  |
  v
Transport::send()
  |
  v
Network

The Client therefore hides serialization and raw network transmission from the application.
If the Client is disconnected, the send operation fails with an appropriate common::Error.

12. Packet Receiving
The Client exposes:

common::Error receive(
    protocol::Packet& packet);

The receive operation reverses the serialization process:

Network
   |
   v
Transport
   |
   v
std::vector<uint8_t>
   |
   v
Decoder
   |
   v
Packet

The resulting Packet is returned through the supplied reference.

13. Encoder and Decoder Integration
The Client reuses the existing protocol modules.
The responsibilities remain separated:
Module	Responsibility
Client	High-level packet communication
Transport	Raw byte transmission
Connection	Connection ownership/state
Socket	OS socket operations
Encoder	Packet → bytes
Decoder	Bytes → Packet
Packet	In-memory protocol representation

The Client must not duplicate serialization logic.

14. Error Handling
Client operations use:common::Error

rather than throwing exceptions for expected operational failures.
Possible errors include:

ConnectionFailed
SocketError
InvalidPacket
SerializationError
DeserializationError

The Client propagates lower-level errors rather than hiding failures.
This provides a consistent error-handling model across PacketForge.

15. Transport Integration
The Client relies on the existing Transport API:

common::Error send(
    const std::vector<std::uint8_t>& data);

common::Error receive(
    std::vector<std::uint8_t>& data,
    std::size_t size);

bool isConnected() const noexcept;

Transport remains responsible for raw byte transfer.
Client remains responsible for converting between protocol Packets and byte buffers.
This separation prevents the Client from becoming a second transport implementation.

16. Architecture

The final Phase 4 architecture is:

                         PacketForge
                             |
              +--------------+--------------+
              |                             |
          Protocol                       Client
              |                             |
     +--------+--------+                    |
     |        |        |                    |
  Packet   Encoder  Decoder                 |
     |        |        |                    |
     +--------+--------+                    |
              |                             |
              +-------------+---------------+
                            |
                         Transport
                            |
                         Connection
                            |
                           Socket
                            |
                          Network

The Client acts as the application-facing boundary over the existing protocol and network layers.

17. Design Principles
Phase 4 follows these principles.

Abstraction
Applications interact with Client rather than lower-level networking primitives.

Separation of concerns
Client, Transport, Connection, Socket, Encoder, Decoder, and Packet retain separate responsibilities.

Reuse
Existing network and protocol components are reused rather than reimplemented.

RAII
Network resources remain owned by RAII-managed objects.

Move-only ownership
Client objects cannot be copied but can be moved.

Explicit error handling
Operational failures are represented through common::Error.

Const correctness
Read-only operations and packet input parameters remain const where appropriate.

18. Phase 4 Completion Criteria

Phase 4 design is considered complete when:

 Client API defined.
 Client owns Transport.
 Client is non-copyable.
 Client is movable.
 Connection management exposed.
 Connection state exposed.
 Packet send API exposed.
 Packet receive API exposed.
 Existing Transport reused.
 Existing Connection reused.
 Existing Packet reused.
 Existing Encoder reused.
 Existing Decoder reused.
 Error handling uses common::Error.