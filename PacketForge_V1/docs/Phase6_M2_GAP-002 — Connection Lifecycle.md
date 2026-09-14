# GAP-002 — Connection Lifecycle

## 1. Objective

The objective of GAP-002 is to verify and establish correct TCP connection lifecycle management for the PacketForge V1 server application.

The lifecycle shall cover the complete sequence from client connection establishment through request processing, peer or application-initiated disconnection, resource cleanup, and server termination.

GAP-002 shall operate within the frozen PacketForge V1 architecture and shall not introduce asynchronous I/O, non-blocking sockets, event loops, multiplexing, or multi-client connection management.

## 2. Design Decision

### DD-M2-002 — Synchronous Connection Lifecycle Management

PacketForge V1 shall manage the TCP client connection through the existing synchronous `network::Connection` and `network::Transport` abstractions.

A connection becomes active only after a successful server-side `accept()` operation. The accepted connection is transferred to the `Transport` object, which owns the connection for the active application transaction.

The connection shall remain active while packet reception and request/response processing are performed. The connection shall be explicitly disconnected during normal completion, peer disconnect, application shutdown, or failure cleanup.

Connection cleanup shall remain centralized in the server application.

## 3. Scope

GAP-002 covers:

* Connection initialization.
* Successful client connection establishment.
* Connection state reporting.
* Ownership transfer of the accepted connection.
* Packet reception over an established connection.
* Detection of peer disconnection.
* Normal request/response lifecycle.
* Client disconnect after response processing.
* Shutdown while a client connection is active.
* Connection cleanup.
* Transport destruction.
* Server cleanup after client termination.
* Idempotent connection cleanup.
* Successful lifecycle exit status.

## 4. Design Constraints

The following PacketForge V1 constraints remain unchanged:

* Single TCP client.
* Single server instance.
* Synchronous blocking socket I/O.
* Standard kernel TCP/IP stack.
* No `select()`.
* No `poll()`.
* No `epoll()`.
* No non-blocking sockets.
* No asynchronous event loop.
* No connection-management thread.
* No simultaneous multi-client handling.
* No dynamic connection scaling.

GAP-002 shall therefore be implemented and validated entirely within the existing synchronous execution model.

## 5. Connection Lifecycle Model

The logical connection lifecycle is:

DISCONNECTED
      │
      │ accept()
      ▼
CONNECTED
      │
      ├───────────────┐
      │               │
      │ receive       │ peer disconnect
      ▼               ▼
RECEIVING      CONNECTION CLOSED
      │               │
      │               ▼
      │           CLEANUP
      │
      ▼
PROCESSING
      │
      ▼
RESPONSE SENT
      │
      ├── client disconnect
      │
      └── application shutdown
               │
               ▼
            CLEANUP
               │
               ▼
          DISCONNECTED

## 6. Connection Ownership

The server initially creates a local `network::Connection` object.

After successful `accept()`, the accepted connection is transferred into `network::Transport`.

Conceptually:
Server
  │
  │ accept()
  ▼
Connection
  │
  │ move ownership
  ▼
Transport
  │
  └── Connection

This ensures that the active connection has a clearly defined owner during packet processing.

The server application retains ownership of the `Transport` through `std::unique_ptr`.

## 7. Connection Establishment

The server waits for a client connection using the existing synchronous `accept()` operation.

After a successful accept:
Client TCP connection
        │
        ▼
server.accept(connection)
        │
        ▼
connection.connected_ = true
        │
        ▼
Connection state = CONNECTED

The application reports the connection state after successful establishment.

A failed `accept()` operation is treated as an application-level failure and results in the appropriate error handling path.

## 8. Active Connection Processing

Once the connection is established:

1. Ownership is transferred to `Transport`.
2. `PacketIO` is constructed using the active transport.
3. The server waits synchronously for a packet.
4. A successfully received packet is dispatched.
5. The generated response is sent through the same active connection.

The normal lifecycle is:
CONNECTED
    │
    ▼
WAITING FOR PACKET
    │
    ▼
PACKET RECEIVED
    │
    ▼
DISPATCH
    │
    ▼
RESPONSE GENERATED
    │
    ▼
RESPONSE SENT

## 9. Peer Disconnect Handling

If the client closes the TCP connection while the server is blocked in `receive()`, the underlying `recv()` operation returns zero.

The transport layer converts this condition into a connection-closed error.

The resulting application flow is:

CONNECTED
    │
    │ client closes socket
    ▼
recv() returns 0
    │
    ▼
Receive failed: connection closed
    │
    ▼
Centralized cleanup

The server then disconnects the active connection, releases the transport, stops the listening server, and terminates normally.

## 10. Normal Request/Response Lifecycle

For a valid request:

Client                         Server
  │                              │
  │ HelloRequest (1)             │
  ├─────────────────────────────>│
  │                              │
  │                         Receive packet
  │                              │
  │                         Dispatch request
  │                              │
  │     HelloResponse (2)        │
  │<─────────────────────────────┤
  │                              │
  │ Client disconnect             │
  ├─────────────────────────────>│
  │                              │
  │                         Cleanup

The same TCP connection is used for both the request and response.

## 11. Application Shutdown with Active Connection

If an application shutdown request is received while a client connection is active, the existing GAP-001 shutdown mechanism is used.

The signal handler only records the shutdown request.

The application-level flow is:
CONNECTED
    │
    │ Ctrl+C
    ▼
Shutdown requested
    │
    ▼
Centralized cleanup
    │
    ├── disconnect active client
    │
    ├── release transport
    │
    └── stop server

GAP-002 therefore integrates with, but does not replace, the GAP-001 graceful shutdown design.

## 12. Centralized Cleanup

All normal and failure exit paths transfer control to the centralized cleanup section of `server_main.cpp`.

Cleanup is performed in the following order:

1. Disconnect the active client connection.
2. Release the `Transport`.
3. Stop the listening server.
4. Return the application exit status.

The cleanup design avoids duplicated resource-management logic across individual error paths.

## 13. Idempotent Cleanup

Connection cleanup is designed to be safe when invoked more than once.

`Connection::disconnect()` closes the socket only when it is open and always resets the connection state to disconnected.

Consequently, repeated cleanup does not result in an invalid socket operation or inconsistent connection state.

The same principle applies to server shutdown through `Server::stop()`.

## 14. GAP-002 Validation Matrix

| ID         | Validation Item                     | Expected Result                                 | Status |
| ---------- | ----------------------------------- | ----------------------------------------------- | ------ |
| GAP-002.1  | Successful connection establishment | Connection becomes `CONNECTED`                  | PASS   |
| GAP-002.2  | Client disconnect before packet     | Server detects closed connection and cleans up  | PASS   |
| GAP-002.3  | Valid request/response lifecycle    | Request received, dispatched, and response sent | PASS   |
| GAP-002.4  | Client disconnect after response    | Active connection is cleaned up                 | PASS   |
| GAP-002.5  | Ctrl+C with active connection       | Active client and server are cleaned up         | PASS   |
| GAP-002.6  | Connection state after disconnect   | Connection returns to disconnected state        | PASS   |
| GAP-002.7  | Transport after cleanup             | No active connection remains                    | PASS   |
| GAP-002.8  | Server state after client cleanup   | Listening server is stopped                     | PASS   |
| GAP-002.9  | Repeated disconnect/cleanup         | Cleanup remains safe and idempotent             | PASS   |
| GAP-002.10 | Lifecycle completion exit status    | Normal completion returns `0`                   | PASS   |

## 15. Validation Evidence

The following runtime behaviors were observed during GAP-002 validation:

### Successful connection

Client connected successfully
Connection state: CONNECTED

### Peer disconnect before packet

Waiting for packet...
Failed to receive packet: Receive failed: connection closed
Client connection closed
PacketForge server stopped successfully

### Normal request/response

Packet received successfully
Received request opcode: 1
Dispatching request...
Request dispatched successfully
Sending response packet...
Response packet sent successfully

### Active connection shutdown

Waiting for packet...
^C
Client connection closed
PacketForge server stopped successfully

### Exit status

echo $?
0

## 16. Gap Assessment
The GAP-002 analysis did not identify a functional deficiency in the existing connection, transport, or server lifecycle implementation.

The existing implementation already provides:

* Explicit connection state.
* Controlled ownership transfer.
* Synchronous connection handling.
* Peer-disconnect detection.
* Centralized cleanup.
* Idempotent disconnect.
* Server cleanup.
* Correct successful exit status.

Therefore, no architectural change is required for GAP-002.

## 17. Final Decision

**GAP-002 — Connection Lifecycle is COMPLETE.**

The PacketForge V1 connection lifecycle is considered validated for the defined single-client synchronous operating model.

No change to the frozen V1 architecture is required as a result of GAP-002.

The validated lifecycle is:

DISCONNECTED
     │
     ▼
CONNECTED
     │
     ▼
REQUEST / RESPONSE
     │
     ├───────────────┐
     │               │
     ▼               ▼
CLIENT DISCONNECT   SHUTDOWN
     │               │
     └───────┬───────┘
             ▼
          CLEANUP
             │
             ▼
        SERVER STOPPED