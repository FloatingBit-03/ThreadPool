# PacketForge V1.0
## Phase 6 — Application & Client–Server Integration
# M2 Shutdown Contract

**Document ID:** PF-V1-P6-M2-SC-001
**Phase:** Phase 6 — Application & Client–Server Integration
**Milestone:** M2 — Server Application
**Gap:** GAP-001 — Graceful Shutdown
**Version:** 1.0
**Status:** Approved for Implementation

## 1. Purpose
This document defines the graceful shutdown contract for the PacketForge V1.0 server application.

The purpose of this contract is to establish a deterministic and well-defined shutdown mechanism for the server application while preserving the existing synchronous network architecture.

The shutdown contract defines:

* application shutdown state
* shutdown request mechanism
* shutdown ownership
* cleanup responsibilities
* connection lifecycle during shutdown
* server lifecycle during shutdown
* error-path cleanup
* interaction with blocking network operations
* centralized cleanup behavior
* shutdown success criteria

This contract applies specifically to Phase 6, Milestone M2 — Server Application.

## 2. Scope
The shutdown contract applies to the following PacketForge V1.0 components:

* Server Application
* Server
* Connection
* Socket
The protocol layer is not responsible for application shutdown.

The following components are outside the scope of this contract:

* Packet
* Encoder
* Decoder
* PacketIO
* Opcode Dispatcher
* Client application
* Worker thread pool
* Multiplexing or event-driven networking

## 3. V1.0 Shutdown Design Principle
PacketForge V1.0 shall retain its existing synchronous network architecture.
The server shall continue to use blocking network operations:

Server::accept()
PacketIO::receive()

No change shall be made to convert these operations into non-blocking or asynchronous operations.

The shutdown mechanism shall therefore operate at the application level and shall not require the networking layer to become event-driven.

A shutdown request does not guarantee immediate interruption of a currently blocked `accept()` or `receive()` operation.

This behavior is an intentional V1.0 architectural constraint.

## 4. Design Decision

### DD-M2-001 — Synchronous Graceful Shutdown

**Decision**
PacketForge V1.0 shall use a simple synchronous graceful shutdown model.
The existing blocking `accept()` and `receive()` operations shall remain unchanged.

An application-level shutdown request shall initiate shutdown processing. The application shall subsequently execute a centralized cleanup sequence that disconnects the active client connection and stops the server.

The signal-handling mechanism shall only record the shutdown request. Resource cleanup shall remain the responsibility of the normal application execution path.

### Rationale
This decision is made to:

1. preserve the existing V1.0 architecture
2. minimize implementation complexity
3. avoid unnecessary changes to the network layer
4. preserve the single-client synchronous execution model
5. avoid introducing multiplexing infrastructure
6. maintain clear module responsibilities
7. provide deterministic resource cleanup
8. keep the M2 implementation focused on server application behavior

### Explicitly Excluded from V1.0
The following mechanisms shall not be introduced as part of GAP-001:

* `epoll`
* `select`
* `poll`
* non-blocking sockets
* event loops
* asynchronous I/O
* asynchronous shutdown frameworks
* socket operations directly from a signal handler
* complex signal-processing logic

## 5. Shutdown Ownership
Shutdown responsibility shall be divided between the application and lower-level components.

### 5.1 Server Application
`server_main.cpp` shall own:

* application shutdown state
* detection of shutdown requests
* application control flow
* decision to terminate
* centralized cleanup sequencing

The application shall request shutdown through the public interfaces of the lower-level components.

### 5.2 Server
The `server::Server` class shall own:

* listening socket
* server running state
* server stop operation
* listening socket cleanup

The server application shall invoke:
server.stop();

The application shall not directly close the listening socket.

### 5.3 Connection
The `network::Connection` class shall own:

* accepted client socket
* connection state
* client connection cleanup

The application shall invoke:
connection.disconnect();

The application shall not directly close the client socket.

### 5.4 Socket
The `network::Socket` class shall remain responsible for low-level socket resource management.

## 6. Shutdown State
The server application shall maintain an explicit application-level shutdown state.

The initial state shall be:
shutdownRequested = false

When a shutdown request is received:
shutdownRequested = true

The conceptual application state transition is:

RUNNING
   |
   | Shutdown Request
   v
SHUTDOWN_REQUESTED
   |
   v
CLEANUP
   |
   v
STOPPED

The shutdown state shall belong to the application layer.

It shall not be introduced into the protocol layer because application shutdown is independent of packet semantics.
## 7. Shutdown Request

For V1.0, the primary external shutdown mechanism shall be:

Ctrl+C
   |
   v
SIGINT
   |
   v
Application Shutdown Request

The signal handler shall perform only the minimum operation required to record that shutdown has been requested.
The signal handler shall not:

* close sockets
* disconnect connections
* invoke `Server::stop()`
* invoke `Connection::disconnect()`
* perform logging
* allocate memory
* execute complex application logic

Resource cleanup shall be performed by the application execution path.

## 8. Application Shutdown Flow

The application shutdown flow shall follow:

Server Running
      |
      v
Shutdown Requested
      |
      v
Application Detects Shutdown
      |
      v
Centralized Cleanup
      |
      +----------------------+
      |                      |
      v                      v
Connection Disconnect     Server Stop
      |                      |
      v                      v
Client Socket Closed     Listening Socket Closed
      |                      |
      +----------+-----------+
                 |
                 v
            Application Exit

## 9. GAP-001.1 — Application Shutdown State
The first implementation requirement is the introduction of an application-level shutdown state.

The state shall:

* represent whether shutdown has been requested
* be initialized to the running state
* be updated when the shutdown request is received
* be checked by the application at appropriate execution boundaries

The shutdown state shall not perform cleanup itself.

Conceptually:
Application Start
      |
      v
shutdownRequested = false
      |
      v
Normal Execution

After a shutdown request:
shutdownRequested = true

The application shall then transition toward cleanup.

## 10. GAP-001.2 — Explicit Shutdown Request
The application shall support an explicit shutdown request through SIGINT.

The expected sequence is:
User presses Ctrl+C
        |
        v
      SIGINT
        |
        v
Shutdown state updated
        |
        v
Application observes request
        |
        v
Cleanup

The signal handler shall remain intentionally minimal.
It shall not own application shutdown or resource destruction.

## 11. GAP-001.3 — Application-Level Shutdown Checks
The application shall check the shutdown state at appropriate execution boundaries.
The expected execution model is:

Application Start
       |
       v
Shutdown Check
       |
       v
Server Start
       |
       v
Shutdown Check
       |
       v
Blocking accept()
       |
       v
Shutdown Check
       |
       v
Blocking receive()
       |
       v
Shutdown Check
       |
       v
Request Processing
       |
       v
Shutdown Check
       |
       v
Response Transmission
       |
       v
Shutdown Check
       |
       v
Cleanup
The existing blocking operations shall remain blocking.

The application shall not introduce polling, multiplexing, or non-blocking socket operations solely to improve shutdown responsiveness.

## 12. Blocking Operation Behavior
PacketForge V1.0 intentionally permits the server to remain blocked inside:

server.accept();

or:
packetIO.receive();

while waiting for network activity.
If a shutdown request occurs while one of these operations is blocked, immediate interruption is not required by the V1.0 shutdown contract.
The application shall complete or exit the blocking operation according to the existing synchronous network behavior before executing its normal cleanup sequence.

This limitation is explicitly accepted as part of the V1.0 design.

## 13. GAP-001.4 — Centralized Cleanup

All application termination paths shall converge on a common cleanup strategy.

The cleanup sequence shall be:
1. Disconnect active client connection
2. Stop PacketForge server
3. Release remaining application-owned resources
4. Return from main()

Conceptually:
              Application Exit
                     |
          +----------+----------+
          |                     |
       Normal                 Error
          |                     |
          +----------+----------+
                     |
                     v
             Centralized Cleanup
                     |
                     v
       Connection::disconnect()
                     |
                     v
               Server::stop()
                     |
                     v
                 main() exit

This prevents individual execution paths from accidentally omitting required cleanup operations.

## 14. Error Shutdown

Runtime errors shall also follow the centralized cleanup strategy.

### Receive Failure

Receive Failure
      |
      v
Record Error
      |
      v
Disconnect Connection
      |
      v
Stop Server
      |
      v
Return Failure

### Dispatch Failure

Dispatch Failure
      |
      v
Record Error
      |
      v
Disconnect Connection
      |
      v
Stop Server
      |
      v
Return Failure

### Send Failure

Send Failure
      |
      v
Record Error
      |
      v
Disconnect Connection
      |
      v
Stop Server
      |
      v
Return Failure

The exact error code returned by `main()` shall remain determined by the existing application error-handling policy.

## 15. Normal Shutdown
After successful request processing and response transmission, the application shall perform orderly connection shutdown.
The normal sequence shall be:

Response Sent
      |
      v
Connection Disconnect
      |
      v
Server Stop
      |
      v
Server Application Exit
The existing V1.0 user-controlled shutdown interaction may remain in place where required by the current application workflow.

## 16. Idempotent Shutdown
Shutdown operations shall remain safe when invoked more than once.

The following operations shall be idempotent:
connection.disconnect();
server.stop();

Calling either operation when the corresponding resource is already closed shall not result in a crash or undefined behavior.
The existing implementation already provides this behavior and shall preserve it during GAP-001 implementation.

## 17. Server Lifecycle Contract

The server lifecycle shall remain:

CREATED
   |
   v
START
   |
   v
RUNNING
   |
   v
LISTENING
   |
   v
CLIENT CONNECTED
   |
   v
PROCESSING
   |
   v
SHUTDOWN
   |
   v
STOPPED

`Server` shall remain responsible for the server-side lifecycle operations:
start()
stop()
isRunning()
accept()

No new server lifecycle abstraction shall be introduced for GAP-001.

## 18. Connection Lifecycle Contract

The client connection lifecycle shall remain:

DISCONNECTED
      |
      | accept()
      v
CONNECTED
      |
      v
COMMUNICATION
      |
      +----------------------+
      |                      |
      v                      v
Normal Completion         Error
      |                      |
      +----------+-----------+
                 |
                 v
             disconnect()
                 |
                 v
            DISCONNECTED
```
`Connection::disconnect()` shall remain responsible for closing the accepted client connection.

## 19. Responsibility Matrix

| Responsibility                  | Server Application | Server | Connection | Socket |
| ------------------------------- | -----------------: | -----: | ---------: | -----: |
| Detect shutdown request         |                Yes |     No |         No |     No |
| Maintain shutdown state         |                Yes |     No |         No |     No |
| Decide application termination  |                Yes |     No |         No |     No |
| Stop accepting clients          |            Request |    Yes |         No |     No |
| Stop server                     |            Request |    Yes |         No |     No |
| Close listening socket          |                 No |    Yes |         No |    Yes |
| Disconnect client               |            Request |     No |        Yes |     No |
| Close client socket             |                 No |     No |        Yes |    Yes |
| Low-level descriptor management |                 No |     No |         No |    Yes |

## 20. Architectural Constraints

GAP-001 implementation shall comply with the following V1.0 constraints:

1. The server remains synchronous.
2. `accept()` remains blocking.
3. `receive()` remains blocking.
4. No multiplexing mechanism shall be introduced.
5. No event loop shall be introduced.
6. No non-blocking socket conversion shall be introduced.
7. Signal handling shall remain minimal.
8. Signal handlers shall not perform socket cleanup.
9. Server owns the listening socket.
10. Connection owns the client socket.
11. Application owns shutdown control flow.
12. Cleanup shall remain idempotent.
13. Existing PacketForge protocol behavior shall not be changed as part of GAP-001.

## 21. GAP-001 Implementation Checklist

### GAP-001.1 — Application Shutdown State

* [ ] Add application shutdown state.
* [ ] Initialize shutdown state to `false`.
* [ ] Keep shutdown state at application level.
* [ ] Do not add shutdown state to protocol/network classes.

### GAP-001.2 — Explicit Shutdown Request

* [ ] Add SIGINT handling.
* [ ] Map Ctrl+C to shutdown request.
* [ ] Keep signal handler minimal.
* [ ] Do not perform resource cleanup from the signal handler.

### GAP-001.3 — Application-Level Shutdown Checks

* [ ] Add shutdown checks at appropriate application boundaries.
* [ ] Preserve blocking `accept()`.
* [ ] Preserve blocking `receive()`.
* [ ] Do not introduce polling or multiplexing.

### GAP-001.4 — Centralized Cleanup

* [ ] Establish one application cleanup path.
* [ ] Disconnect active client connection.
* [ ] Stop PacketForge server.
* [ ] Preserve idempotent cleanup behavior.
* [ ] Ensure normal and error paths use the cleanup strategy.

## 22. Verification Criteria

GAP-001 shall be considered complete only when all of the following are satisfied:

* [ ] Server builds successfully.
* [ ] Existing unit tests pass.
* [ ] Server starts successfully.
* [ ] Server enters listening state.
* [ ] Client can connect.
* [ ] Normal server termination performs connection cleanup.
* [ ] Server stop closes the listening socket.
* [ ] Client connection is disconnected cleanly.
* [ ] SIGINT generates an application shutdown request.
* [ ] Shutdown does not perform socket operations from the signal handler.
* [ ] Error paths perform cleanup.
* [ ] Repeated `stop()` and `disconnect()` calls remain safe.
* [ ] No existing V1.0 protocol behavior is changed.
* [ ] No multiplexing or asynchronous networking infrastructure is introduced.

## 23. Expected Final Shutdown Flow

The final V1.0 M2 shutdown architecture shall be:

                         PacketForge Server
                                |
                                v
                         Application Running
                                |
                   +------------+------------+
                   |                         |
             Normal Flow                SIGINT / Ctrl+C
                   |                         |
                   |                         v
                   |                Shutdown Requested
                   |                         |
                   +------------+------------+
                                |
                                v
                     Application Shutdown Check
                                |
                                v
                       Centralized Cleanup
                                |
                    +-----------+-----------+
                    |                       |
                    v                       v
          Connection::disconnect()     Server::stop()
                    |                       |
                    v                       v
             Client Socket Closed    Listening Socket Closed
                    |                       |
                    +-----------+-----------+
                                |
                                v
                         Application Stopped
                                |
                                v
                           main() returns
```
## 24. Final Contract
PacketForge V1.0 shall implement graceful shutdown as an **application-level synchronous shutdown mechanism**.

The networking architecture shall remain unchanged.The application shall detect a shutdown request, transition to the shutdown state, and execute centralized cleanup through the existing `Connection::disconnect()` and `Server::stop()` interfaces.
The shutdown mechanism shall prioritize architectural simplicity, deterministic cleanup, clear ownership, and preservation of the V1.0 synchronous design.

**GAP-001 shall therefore be implemented through the following four controlled changes:**

GAP-001.1  Application Shutdown State
      ↓
GAP-001.2  Explicit Shutdown Request
      ↓
GAP-001.3  Application-Level Shutdown Checks
      ↓
GAP-001.4  Centralized Cleanup

No additional networking architecture shall be introduced as part of this milestone.
