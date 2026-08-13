# PacketForge — Phase 5 Development Notes

## 1. Phase Objective
The objective of Phase 5 was to introduce the Server layer on top of
the existing networking primitives.

Before Phase 5, PacketForge already provided:

- Error
- Socket
- Connection
- Transport
- Protocol components
- Client

Phase 5 adds the corresponding server-side lifecycle abstraction.

## 2. Implementation Order
The Server implementation was developed in the following order:

Server API
    |
    v
Server state
    |
    v
start()
    |
    +--> create socket
    +--> bind address/port
    +--> listen
    +--> retrieve actual port
    +--> mark running
    |
    v
stop()
    |
    v
accept()
    |
    v
move semantics
    |
    v
unit tests
    |
    v
Makefile integration

3. Initial Server State
The constructor initializes:

Server::Server()
    :
    socket_{},
    running_{false},
    port_{0}
{
}

This guarantees that a newly-created Server does not own an active listening socket.

The initial state is therefore:
socket = closed
running = false
port = 0

4. Implementing start()
The start() operation was implemented as a sequence of existing
Socket operations.

Step 1 — Check lifecycle state
if (running_)
{
    return Error(...);
}

This prevents starting an already-running Server.

Step 2 — Create socket
auto error = socket_.create();

If creation fails, the error is returned immediately.

Step 3 — Bind
error = socket_.bind(
    address,
    port
);

If binding fails, the socket is explicitly closed before returning.

Step 4 — Listen
error = socket_.listen(
    SOMAXCONN
);

Again, failure closes the socket.

Step 5 — Retrieve actual port

getsockname() is used to determine the actual bound port.

This is necessary when the caller supplies:

port == 0

The resulting port is converted from network byte order:

port_ = ntohs(
    addressInfo.sin_port
);
Step 6 — Mark Server running

Only after all previous operations succeed:

running_ = true;

This ordering is important.

The Server must never report itself as running before the complete socket setup succeeds.

5. Implementing stop()
stop() was designed to be safe regardless of the current lifecycle state.

The implementation closes the socket when necessary:
if (socket_.isOpen())
{
    socket_.close();
}

Then it resets the logical state:
running_ = false;
port_ = 0;

This makes repeated calls safe.

6. Implementing accept()

The Server checks its lifecycle state before accepting:
if (!running_)
{
    return common::Error(
        common::ErrorCode::ConnectionFailed,
        "Server is not running"
    );
}

If the Server is running, it delegates the actual client acceptance to the existing Connection abstraction:
return connection.accept(
    socket_
);

This avoids duplicating low-level accept logic inside the Server.

7. Implementing Move Construction
The Server owns a socket resource, so copying would be unsafe.
Move construction transfers the socket:
socket_(
    std::move(other.socket_)
)

and transfers the logical state:
running_(
    other.running_
),
port_(
    other.port_
)

The source Server is then reset:
other.running_ = false;
other.port_ = 0;

The underlying Socket move operation transfers the actual native socket ownership.

8. Implementing Move Assignment
Move assignment first releases any resource already owned by the destination:
stop();

It then transfers:
socket_
running_
port_

from the source object.
Finally, the source object is reset to an inactive state. This follows the same ownership model as the rest of the networking layer.

9. Error Recovery
A major implementation requirement was that failed startup must not leak a socket.

The following operations can fail:
create
bind
listen
getsockname

For failures after socket creation:
failure
   |
   v
close socket
   |
   v
return Error

Therefore the Server does not remain partially initialized after a failed start().

10. Testing Development
Testing was performed incrementally rather than waiting until then entire Server implementation was complete.
The Server test suite was designed around the public API.

The final suite contains 12 tests:
ServerStartsStopped
StartSucceeds
StopStopsServer
StopMultipleTimesIsSafe
StartWithInvalidAddressFails
StartWithInvalidPortFails
StartWhenAlreadyRunningFails
AcceptFailsWhenServerIsStopped
AcceptSucceeds
ServerRemainsRunningAfterAccept
MoveConstructorTransfersOwnership
MoveAssignmentTransfersOwnership

11. Network Testing Strategy
Real loopback sockets are used for Server tests.

The tests use: 127.0.0.1
rather than external network interfaces.

When possible, port 0 is used so that the operating system selects an available port.

The test then obtains the actual port using: server.port()
This avoids hard-coding a potentially occupied port.

12. Accept Test Strategy
The successful accept test follows this sequence:

Server
  |
  | start("127.0.0.1", 0)
  |
  v
Listening
  |
  | obtain server.port()
  |
  v
Client socket connects
  |
  v
Server.accept(connection)
  |
  v
Connection becomes connected

The test verifies that: 
error.ok()
connection.isConnected(), are both true.

13. Move Testing
Move tests verify both sides of ownership transfer.

For move construction:
Server moved(
    std::move(server)
);

The test verifies that:
moved
 └── owns listening socket

server
 └── no longer owns listening socket

Move assignment is tested similarly.
These tests are important because socket ownership is a resource management concern rather than simply a value-copying operation.

14. Build System Integration
The Server implementation is compiled into:

build/server/server.o

The Server test is compiled into:
build/tests/test_server.o

The Server test executable links:
build/server/server.o
build/network/socket.o
build/network/connection.o
build/common/error.o
build/tests/test_server.o

along with:
-lgtest
-lgtest_main
-pthread

The resulting executable is:
build/tests/test_server

15. Phase 5 Verification
The complete unit-test suite was executed using: make test

The Server test executable reports: [==========] Running 12 tests from 1 test suite.
...
[  PASSED  ] 12 tests.

The complete project test suite also passes.

16. Current Limitations

The Phase 5 Server is intentionally a basic synchronous server.

It currently does not provide:

Concurrent client handling
Multiple worker threads
Thread-pool integration
Asynchronous accept
Connection storage
Client/session tracking
Packet dispatch
Server event loop
Graceful shutdown of active clients

These are future concerns and should not be added to the Phase 5 Server solely for the sake of increasing functionality.

17. Design Principle
The primary design principle of Phase 5 is:

Keep the Server responsible for server lifecycle and listening-socket ownership, while delegating low-level socket and connection operations to the existing networking abstractions.

This keeps the Server API small and allows later phases to build higher-level functionality on top of it.

# PacketForge — Phase 5 Server Testing

## Test Suite

Phase 5 contains 12 unit tests for the `Server` API.

| Test | Purpose |
|---|---|
| `ServerStartsStopped` | Verifies initial lifecycle state |
| `StartSucceeds` | Verifies successful server startup |
| `StopStopsServer` | Verifies shutdown |
| `StopMultipleTimesIsSafe` | Verifies idempotent shutdown |
| `StartWithInvalidAddressFails` | Verifies invalid address handling |
| `StartWithInvalidPortFails` | Verifies invalid port handling |
| `StartWhenAlreadyRunningFails` | Verifies lifecycle protection |
| `AcceptFailsWhenServerIsStopped` | Verifies invalid accept state |
| `AcceptSucceeds` | Verifies real client acceptance |
| `ServerRemainsRunningAfterAccept` | Verifies listening socket remains active |
| `MoveConstructorTransfersOwnership` | Verifies move construction |
| `MoveAssignmentTransfersOwnership` | Verifies move assignment |

## Verification

Run: make test
The Phase 5 Server suite must finish with: [  PASSED  ] 12 tests.

The complete project test suite must also pass.
Important Test Properties
Lifecycle
constructed
    ↓
stopped
    ↓
start()
    ↓
running
    ↓
stop()
    ↓
stopped
Port allocation

Tests use port 0 where dynamic allocation is useful.

The actual port is obtained through: server.port()

Networking
Successful connection tests use the loopback interface: 127.0.0.1

This keeps the tests local and avoids depending on external network connectivity.

Ownership
Move tests verify that the source object no longer owns the listening socket after ownership transfer.


### Phase 5 documentation boundary

I would **not** put these things into Phase 5 documentation yet:

- Thread-pool architecture
- Async server design
- Packet routing
- Session manager
- Multi-client architecture
- Production deployment
- Performance benchmarks
- Future protocol design in detail

Those belong to later phases.

For Phase 5, the documentation should essentially answer four questions:

WHAT was designed?
      ↓
Server lifecycle + ownership + API

WHY was it designed this way?
      ↓
Layer separation + RAII + move-only resources

HOW was it developed?
      ↓
Socket → bind → listen → port → accept → lifecycle

HOW was it verified?
      ↓
12 Server tests + complete make test