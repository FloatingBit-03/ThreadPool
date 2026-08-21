PacketForge — Phase 6 Design & Development Roadmap

Phase: 6 — Application & Client–Server Integration
Objective: Assemble the completed PacketForge library modules into a functional client/server application and establish the first complete end-to-end communication path.

1. Phase 6 Overview

Phase 6 is the application phase of PacketForge.The previous phases established the reusable library components:

Phase 1
Common
    |
    v
Phase 2
Network
    |
    v
Phase 3
Protocol
    |
    +----------------+
    |                |
    v                v
Phase 4            Phase 5
Client             Server

Phase 6 brings these components together:

                    Phase 6
              Application Layer
                     |
          +----------+----------+
          |                     |
          v                     v
   Client Application    Server Application
          |                     |
          v                     v
       Client                 Server
          |                     |
          +----------+----------+
                     |
                  Network
                     |
                  Protocol

The purpose of Phase 6 is therefore not to introduce another low-level networking abstraction.
Its purpose is to prove that the existing abstractions can be assembled into a working system.

2. Primary Phase 6 Goal
The primary goal is to produce two executable applications:

packetforge_client
packetforge_server

which can communicate using the PacketForge networking and protocol stack.

The first complete system should support:
Server starts
     |
     v
Server listens
     |
     v
Client starts
     |
     v
Client connects
     |
     v
Server accepts
     |
     v
Client sends request
     |
     v
Server receives request
     |
     v
Server processes request
     |
     v
Server sends response
     |
     v
Client receives response
     |
     v
Clean shutdown

This becomes the first genuine end-to-end validation of PacketForge.

3. Phase 6 Design Principles
Phase 6 should follow several strict principles.

3.1 Do not duplicate library functionality
Applications must use the existing classes.

For example:
Server server;
Client client;

rather than directly using:
socket()
bind()
listen()
connect()
send()
recv()

inside main().

The operating-system networking API belongs below the application layer.

3.2 Applications orchestrate; libraries implement
The distinction should be:
Library
    |
    +-- provides functionality


Application
    |
    +-- decides how functionality is used

For example:

Server
    "I know how to start listening."

server_main
    "I want to start the server and run it."

Similarly:

Client
    "I know how to communicate."

client_main
    "I want to communicate with this server."

4. Phase 6 Scope
Phase 6 is divided into the following development stages:

6.1  Application Architecture
          |
          v
6.2  Application Entry Points
          |
          v
6.3  Server Application
          |
          v
6.4  Client Application
          |
          v
6.5  Client–Server Connection Integration
          |
          v
6.6  Packet Exchange Integration
          |
          v
6.7  Request/Response Flow
          |
          v
6.8  Error and Shutdown Handling
          |
          v
6.9  Integration Test Suite
          |
          v
6.10 Build System Integration
          |
          v
6.11 End-to-End Validation
          |
          v
6.12 Phase 6 Documentation & Release Review

Each stage should be completed before moving to the next.

5. Phase 6.1 — Application Architecture
Objective
Define the application layer before writing executable code.

The application structure should be:

apps/
├── client_main.cpp
└── server_main.cpp

The library remains:

include/
├── common/
├── network/
├── protocol/
├── client/
└── server/

src/
├── common/
├── network/
├── protocol/
├── client/
└── server/

Testing remains:

tests/
├── unit/
└── integration/
Responsibility Separation

The architecture should follow:

                    Applications
                         |
              +----------+----------+
              |                     |
        Client Application    Server Application
              |                     |
            Client                 Server
              |                     |
          Connection            Connection
              |                     |
          Transport              Transport
              |                     |
           Protocol              Protocol

Applications should not bypass these layers.

6. Phase 6.2 — Application Entry Points
Create:

apps/client_main.cpp
apps/server_main.cpp

Initially, these applications should remain intentionally simple.

Server application

Conceptually:

int main()
{
    packetforge::server::Server server;

    auto error =
        server.start("127.0.0.1", 8080);

    if (!error.ok())
    {
        return 1;
    }

    // Server runtime

    server.stop();

    return 0;
}
Client application

Conceptually:

int main()
{
    packetforge::client::Client client;

    auto error =
        client.connect(
            "127.0.0.1",
            8080
        );

    if (!error.ok())
    {
        return 1;
    }

    // Client runtime

    client.disconnect();

    return 0;
}

These are architectural examples rather than final implementations.

7. Phase 6.3 — Server Application

The server application becomes the runtime owner of the server.

Its initial responsibilities are:

Read configuration
       |
       v
Create Server
       |
       v
Start Server
       |
       v
Wait for clients
       |
       v
Accept connection
       |
       v
Perform application work
       |
       v
Shutdown

The application should not take over responsibilities that belong to Server.

For example, this is undesirable:

::socket(...);
::bind(...);
::listen(...);

inside server_main.cpp.

The correct flow is:

Server server;

server.start(...);
server.accept(...);

8. Phase 6.4 — Client Application
The client application controls the client runtime.
Initial responsibilities:

Read configuration
       |
       v
Create Client
       |
       v
Connect
       |
       v
Create request
       |
       v
Send request
       |
       v
Receive response
       |
       v
Shutdown

Again, the application should use the Client abstraction rather than implementing socket operations itself.

9. Phase 6.5 — Client–Server Connection Integration
This is the first integration milestone.

The goal is:

Server
  |
  | start()
  v
Listening
  |
  |
  | <--------- TCP --------->
  |
  v
Client
  |
  | connect()
  v
Connected

The expected sequence is:

1. Start server
2. Retrieve server port
3. Start client
4. Client connects
5. Server accepts
6. Verify both sides

At this stage we do not yet need complex packet processing.

The purpose is to prove:

Server
    +
Client
    +
Connection
    +
Socket

work correctly together.

10. Phase 6.6 — Packet Exchange Integration

Once connection integration is proven, protocol communication is introduced.

The first data path should be:

Client
   |
   | Packet
   v
Encoder
   |
   v
Transport
   |
   v
TCP Connection
   |
   v
Server
   |
   v
Transport
   |
   v
Decoder
   |
   v
Packet

This validates the complete outbound/inbound stack.

11. Phase 6.7 — Request/Response Flow
The next milestone is a complete request/response transaction.

The flow becomes:

                   CLIENT
                     |
                     |
               Create Request
                     |
                     v
                  Encoder
                     |
                     v
                  Transport
                     |
                     |
                     | TCP
                     |
                     v
                   SERVER
                     |
                     v
                  Transport
                     |
                     v
                  Decoder
                     |
                     v
               Process Request
                     |
                     v
               Create Response
                     |
                     v
                  Encoder
                     |
                     v
                  Transport
                     |
                     |
                     | TCP
                     |
                     v
                   CLIENT
                     |
                     v
                  Decoder
                     |
                     v
              Validate Response

This is the most important technical milestone of Phase 6.

At this point, PacketForge is no longer just a collection of tested modules.

It is functioning as an actual communication framework.

12. Application Message Model
For the first Phase 6 implementation, keep the application protocol extremely simple.

For example:

Request
    opcode = REQUEST
    payload = "Hello Server"

and:

Response
    opcode = RESPONSE
    payload = "Hello Client"

The purpose is not to create a sophisticated application protocol yet.

The purpose is to prove:

Packet
   ↓
Encode
   ↓
Transport
   ↓
Network
   ↓
Transport
   ↓
Decode
   ↓
Packet

works end to end.

13. Phase 6.8 — Error Handling and Shutdown
A real application must handle failure.

Important scenarios include:

Server startup failure
start()
   |
   +-- failure
        |
        v
   Report error
        |
        v
   Exit cleanly
Client connection failure
connect()
   |
   +-- failure
        |
        v
   Report error
        |
        v
   Exit cleanly
Server stopped before accept
accept()
   |
   v
ConnectionFailed
Client disconnect

The application must ensure resources are released correctly.

Server shutdown

The server should:

Stop accepting new connections
        |
        v
Close listening socket
        |
        v
Release resources
        |
        v
Exit

14. Phase 6.9 — Integration Test Suite
Unit tests already verify individual components.

Phase 6 introduces:

tests/integration/

The first integration test should validate:

Server starts
     ↓
Client connects
     ↓
Server accepts
     ↓
Connection established
     ↓
Shutdown

A later test should validate:

Server starts
     ↓
Client connects
     ↓
Client sends packet
     ↓
Server receives packet
     ↓
Server decodes packet
     ↓
Server creates response
     ↓
Server sends response
     ↓
Client receives response
     ↓
Client decodes response

15. Integration Test Categories

The Phase 6 integration suite should eventually cover:

Connection
Server starts successfully
Client connects
Server accepts client
Server remains operational
Communication
Client sends request
Server receives request
Server sends response
Client receives response
Protocol
Packet survives network transmission
Payload remains intact
Opcode remains intact
Sequence information remains correct
Failure
Client cannot connect to stopped server
Invalid server endpoint fails
Connection failure is reported
Disconnect is handled safely
Lifecycle
Server starts
Server stops
Client connects and disconnects
Resources are released

16. Phase 6.10 — Build System Integration

The existing Makefile currently builds:

build/tests/test_*

Phase 6 introduces application binaries.

For example:

build/apps/
├── packetforge_server
└── packetforge_client

The build dependency structure should reflect the actual architecture.

Server application

Conceptually:

server_main.o
    |
    +-- server.o
    +-- connection.o
    +-- socket.o
    +-- transport.o
    +-- packet.o
    +-- encoder.o
    +-- decoder.o
    +-- common objects
Client application
client_main.o
    |
    +-- client.o
    +-- connection.o
    +-- socket.o
    +-- transport.o
    +-- packet.o
    +-- encoder.o
    +-- decoder.o
    +-- common objects

The exact dependency list should be based on what the final application actually uses.

17. Phase 6.11 — End-to-End Validation
At the end of Phase 6, the following scenario should work:

Terminal 1
$ ./packetforge_server
PacketForge server started
Listening on 127.0.0.1:<port>
Waiting for client...
Terminal 2
$ ./packetforge_client
Connecting...
Connected
Sending request...
Response received
Server
Client connected
Request received
Request decoded
Response sent

The exact logging format can evolve later.
The important point is that a real request travels through the entire system.

18. Final Phase 6 Architecture

The target architecture is:

                         PacketForge
                              |
             +----------------+----------------+
             |                                 |
             v                                 v
      Client Application                 Server Application
             |                                 |
             v                                 v
          Client                            Server
             |                                 |
             v                                 v
        Connection                        Connection
             |                                 |
             v                                 v
         Transport                         Transport
             |                                 |
             +---------------+-----------------+
                             |
                           TCP
                             |
             +---------------+-----------------+
             |                                 |
             v                                 v
          Encoder                          Decoder
             |                                 |
             v                                 v
           Packet                           Packet

The actual runtime interaction is:

                REQUEST
Client ----------------------------> Server
       Packet → Encode → Transport

                RESPONSE
Client <---------------------------- Server
       Decode ← Packet ← Transport

19. What Phase 6 Should NOT Include

To keep the phase maintainable, the following should not be part of the initial Phase 6 implementation:

❌ epoll
❌ event loops
❌ thread pools
❌ connection pools
❌ encryption
❌ authentication
❌ compression
❌ database integration
❌ complex configuration framework
❌ distributed deployment
❌ performance optimization

These are future capabilities.

The first goal is a correct synchronous end-to-end implementation.

20. Future Expansion Boundary

Once Phase 6 is complete, the architecture should allow future development without rewriting the existing foundation.

Possible future work:

PacketForge v1.x
    |
    +-- Multiple clients
    +-- Concurrent connections
    +-- Thread pool
    +-- Connection manager
    +-- Event-driven server
    +-- epoll
    +-- Timeouts
    +-- Authentication
    +-- Encryption
    +-- Performance optimization

The important design goal is:

Phase 6 establishes correctness first. Future phases establish scalability and performance.

21. Phase 6 Completion Criteria

Phase 6 is complete when all of the following are satisfied.

Architecture
 Application directory established
 Client and server entry points defined
 Library/application boundary documented
 Application responsibilities clearly separated
Server Application
 Server executable builds
 Server starts successfully
 Server listens on configured endpoint
 Server accepts client connections
 Server shuts down cleanly
Client Application
 Client executable builds
 Client connects to server
 Client sends requests
 Client receives responses
 Client shuts down cleanly
Protocol Integration
 Request packet encoded
 Request transmitted
 Request decoded
 Response generated
 Response encoded
 Response transmitted
 Response decoded
Integration Testing
 Client/server connection test
 Request/response test
 Invalid connection test
 Shutdown test
 Resource cleanup test
Build
 make test passes
 Client application builds
 Server application builds
 Integration tests build
 Full project builds cleanly
Documentation
 Phase 6 design document
 Phase 6 development document
 Application usage documentation
 Architecture updated
 Final Phase 6 completion report

22. Phase 6 Development Workflow
To prevent Phase 6 from becoming too large, development should follow this exact sequence:

                PHASE 6
                   |
                   v
        ┌─────────────────────┐
        │ 6.1 Architecture    │
        └──────────┬──────────┘
                   v
        ┌─────────────────────┐
        │ 6.2 Entry Points    │
        └──────────┬──────────┘
                   v
        ┌─────────────────────┐
        │ 6.3 Server App      │
        └──────────┬──────────┘
                   v
        ┌─────────────────────┐
        │ 6.4 Client App      │
        └──────────┬──────────┘
                   v
        ┌─────────────────────┐
        │ 6.5 Connection      │
        └──────────┬──────────┘
                   v
        ┌─────────────────────┐
        │ 6.6 Packet Exchange │
        └──────────┬──────────┘
                   v
        ┌─────────────────────┐
        │ 6.7 Request/Response │
        └──────────┬──────────┘
                   v
        ┌─────────────────────┐
        │ 6.8 Error/Shutdown  │
        └──────────┬──────────┘
                   v
        ┌─────────────────────┐
        │ 6.9 Integration Test│
        └──────────┬──────────┘
                   v
        ┌─────────────────────┐
        │ 6.10 Build System   │
        └──────────┬──────────┘
                   v
        ┌─────────────────────┐
        │ 6.11 End-to-End     │
        └──────────┬──────────┘
                   v
        ┌─────────────────────┐
        │ 6.12 Documentation  │
        └──────────┬──────────┘
                   v
             PHASE 6 DONE
                   |
                   v
            PacketForge v1.0

23. Final Design Goal
The final Phase 6 objective can be summarized as:

                 PACKETFORGE v1.0
                        |
              +---------+---------+
              |                   |
              v                   v
           CLIENT              SERVER
         APPLICATION         APPLICATION
              |                   |
              v                   v
           Client               Server
              |                   |
              +--------+----------+
                       |
                    Transport
                       |
                     TCP
                       |
                    Protocol
                       |
              Request / Response

The key principle for this phase is:Do not build more infrastructure. Assemble and validate the infrastructure that has already been built.