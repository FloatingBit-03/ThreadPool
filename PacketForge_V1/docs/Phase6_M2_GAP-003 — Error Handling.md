# GAP-003 — Error Handling

## 1. Objective
The objective of GAP-003 is to establish and validate a consistent error-handling strategy for PacketForge V1.

The error-handling mechanism shall ensure that errors are:

* detected at the appropriate layer,
* represented using the common error model,
* propagated to the application boundary,
* classified according to severity,
* logged with sufficient information,
* handled without leaving resources in an inconsistent state, and
* associated with an appropriate application exit status.

GAP-003 shall operate within the frozen PacketForge V1 synchronous architecture.
## 2. Design Decision

### DD-M2-003 — Layered Error Handling and Error Classification

PacketForge V1 shall use a layered error-handling model based on `common::Error`.

Lower-level modules shall detect and return errors to their callers. The application layer shall classify the error and determine the appropriate action.

Errors shall be divided into two principal categories:

### Expected / Handled Conditions

These conditions represent a normal or recoverable protocol/lifecycle event:

* `ConnectionClosed`
* `ProtocolError`

These conditions shall be handled without treating them as PacketForge server infrastructure failures.

### Fatal Conditions

These conditions indicate a server, communication, or processing failure:

* startup failure,
* accept failure,
* socket/system failure,
* response send failure,
* dispatcher failure,
* other unexpected transport or application failures.

Fatal conditions shall result in a non-zero process exit status after centralized cleanup.

## 3. Error Model

PacketForge V1 uses the `common::Error` abstraction with an `ErrorCode` enumeration.

The defined error categories are:

Success
UnknownError
InvalidArgument
ConnectionFailed
ConnectionClosed
SocketError
Timeout
InvalidPacket
SerializationError
DeserializationError
ProtocolError

The `Error` object provides:

* error code,
* error message,
* success-state inspection through `ok()`.

The model allows lower-level components to return structured error information without directly controlling application termination.

## 4. Error Classification

The following classification is used by PacketForge V1:

| Error Condition                | Classification         | Exit Status |
| ------------------------------ | ---------------------- | ----------: |
| Successful operation           | Normal                 |         `0` |
| Client closes connection       | Expected               |         `0` |
| Protocol decoding error        | Handled protocol fault |         `0` |
| Unsupported protocol version   | Handled protocol fault |         `0` |
| Unsupported opcode             | Handled protocol fault |         `0` |
| Server startup failure         | Fatal                  |         `1` |
| Accept failure                 | Fatal                  |         `1` |
| Socket/system error            | Fatal                  |         `1` |
| Response send failure          | Fatal                  |         `1` |
| Dispatcher failure             | Fatal                  |         `1` |
| Unexpected application failure | Fatal                  |         `1` |

This classification prevents expected client/protocol conditions from being incorrectly reported as server infrastructure failures.

## 5. Layered Error Propagation

The error flow is:
System Call / Module
        │
        ▼
common::Error
        │
        ▼
Calling Component
        │
        ▼
Application Boundary
        │
        ├── Expected / Handled
        │       │
        │       ▼
        │    Cleanup
        │       │
        │       ▼
        │     exit 0
        │
        └── Fatal
                │
                ▼
             Log error
                │
                ▼
             Cleanup
                │
                ▼
              exit 1
The lower layer does not directly terminate the process.

## 6. Socket and Server Startup Errors

Socket operations such as:

* `socket()`,
* `bind()`,
* `listen()`,
* `accept()`, and
* `getsockname()`

return errors through the common error model.

The server startup sequence is:

create()
   ↓
bind()
   ↓
listen()
   ↓
getsockname()
   ↓
running = true

If any startup stage fails, the server releases the socket resource, resets its state, propagates the error, and terminates with a non-zero status.

The server therefore does not remain partially initialized after startup failure.

## 7. Connection Closed Handling

A TCP peer closure is distinguished from an actual socket/system failure.

When `recv()` returns zero:

recv() == 0
     ↓
ConnectionClosed

This represents orderly peer termination rather than a server fault.

The application handles this condition as:

ConnectionClosed
     ↓
No fatal error reported
     ↓
Centralized cleanup
     ↓
exitCode = 0

This distinction prevents normal client termination from being reported as a server failure.

## 8. Transport Errors

A negative result from the underlying `recv()` or `send()` system call represents an actual transport/system failure.

For example:
recv() < 0
    ↓
SocketError

and:
send() < 0
    ↓
SocketError

These errors are propagated to the application and treated as fatal unless a specific recovery policy exists.

The current V1 policy is:

Transport failure
      ↓
Log error
      ↓
Centralized cleanup
      ↓
exitCode = 1

## 9. Packet I/O Error Handling

`PacketIO` provides the boundary between transport operations and protocol processing.

### Send path

Packet
   ↓
Encoder
   ↓
Transport::send()

Encoding exceptions are converted to appropriate `common::Error` values.

Transport failures are propagated without being hidden or reclassified unnecessarily.

### Receive path

Transport::receive()
       ↓
Header
       ↓
Payload
       ↓
Decoder

Transport errors are returned to the caller.

Protocol decoding failures are converted to `ErrorCode::ProtocolError` and, where applicable, the specific protocol error is made available separately.

## 10. Protocol Error Handling

A malformed or invalid protocol packet is treated as a client/protocol error rather than a server infrastructure failure.

The flow is:
Malformed Packet
      ↓
Decoder
      ↓
ProtocolDecodeError
      ↓
ErrorCode::ProtocolError
      ↓
Server constructs ErrorResponse
      ↓
ErrorResponse sent
      ↓
Cleanup
      ↓
exitCode = 0

The server therefore acknowledges the protocol failure explicitly through the defined protocol error response mechanism.

If the server cannot transmit that error response, the condition becomes a fatal communication failure.

## 11. Protocol Error Response Failure

The protocol-error response itself is transmitted through the normal `PacketIO` path.

The policy is:
Protocol Error
      │
      ▼
Create ErrorResponse
      │
      ▼
Send ErrorResponse
      │
      ├── success ──► exit 0
      │
      └── failure ──► exit 1

Therefore, successful detection and reporting of a protocol error is not treated as a server failure, but inability to communicate the error response is treated as fatal.

## 12. Dispatcher Error Handling

The `OpcodeDispatcher` is responsible for protocol-level request interpretation.

Examples include:
UnsupportedVersion
UnsupportedOpcode

These are represented as protocol error responses rather than infrastructure failures.

For example:
HelloResponse received as request
        ↓
UnsupportedOpcode
        ↓
ErrorResponse

The dispatcher therefore separates protocol semantics from transport/system failures.

An actual internal dispatcher failure returned through `common::Error` is treated by the application as fatal:

Dispatcher failure
      ↓
Log error
      ↓
Cleanup
      ↓
exitCode = 1

## 13. Error Logging

Error logging is performed at the application boundary after an error has been classified.

Expected protocol/lifecycle events are not incorrectly logged as fatal infrastructure failures.

Examples:
### Expected peer closure

Client connection closed

### Fatal receive failure

Failed to receive packet: <error>

### Startup failure

Failed to start PacketForge server: <error>


### Response send failure

Failed to send response packet: <error>

This classification keeps operational logs meaningful and prevents normal lifecycle events from appearing as failures.

## 14. Centralized Cleanup

All error paths eventually reach the centralized cleanup section in `server_main.cpp`.

Cleanup order:
1. Disconnect active client connection
2. Release Transport
3. Stop listening server
4. Return application exit status

The cleanup mechanism is shared by:

* normal completion,
* connection closure,
* protocol errors,
* startup failures,
* transport failures,
* response-send failures,
* shutdown requests.

This prevents individual error branches from implementing different resource-management behavior.

## 15. Error Handling and Exit Status

The final V1 exit-status policy is:

Expected / handled condition
        ↓
exitCode = 0

Fatal condition
        ↓
exitCode = 1

This provides a deterministic result for automated testing and process supervision.

## 16. GAP-003 Validation Matrix

| ID         | Validation Scenario                    | Expected Result                              | Status |
| ---------- | -------------------------------------- | -------------------------------------------- | ------ |
| GAP-003.1  | Normal request/response regression     | Request succeeds, response sent, exit `0`    | PASS   |
| GAP-003.2  | Client closes connection before packet | `ConnectionClosed`, cleanup, exit `0`        | PASS   |
| GAP-003.3  | Server startup/bind failure            | Error reported, exit `1`                     | PASS   |
| GAP-003.4  | Protocol decode failure                | Protocol error detected                      | PASS   |
| GAP-003.5  | Protocol ErrorResponse generation      | ErrorResponse created and sent               | PASS   |
| GAP-003.6  | Protocol-error success path            | Cleanup, exit `0`                            | PASS   |
| GAP-003.7  | Response send failure                  | Error reported, cleanup, exit `1`            | PASS   |
| GAP-003.8  | Dispatcher failure path                | Fatal error policy defined and code reviewed | PASS   |
| GAP-003.9  | Cleanup after fatal error              | Resources released                           | PASS   |
| GAP-003.10 | Error classification consistency       | Expected vs fatal behavior consistent        | PASS   |


## 17. Validation Evidence

### Connection closed

Waiting for packet...
Client connection closed
PacketForge server stopped successfully

Exit status: 0

### Startup failure

Failed to start PacketForge server: Address already in use

Exit status: 1

### Protocol error

Protocol error detected
Protocol error response sent successfully
Client connection closed
PacketForge server stopped successfully

Fault-injection client confirmed:
Response opcode: 100
Protocol error code: 1

Exit status:0

### Response send failure

Sending response packet...
Failed to send response packet: Send failed
Client connection closed
PacketForge server stopped successfully

Exit status:1

## 18. Final GAP Assessment

The GAP-003 analysis identified and addressed the key error-handling distinction between expected lifecycle/protocol conditions and fatal server failures.

The principal implementation changes were:

* introduction of `ConnectionClosed`,
* mapping `recv() == 0` to `ConnectionClosed`,
* explicit `ConnectionClosed` handling in `server_main.cpp`,
* treating successfully handled protocol errors as non-fatal,
* retaining non-zero exit status for actual transport and response-send failures,
* preserving centralized cleanup.

No architectural change to the frozen PacketForge V1 design was required.

## 19. Final Decision

**GAP-003 — Error Handling is COMPLETE.**

PacketForge V1 now has a defined and validated error-handling strategy covering:

Detection
   ↓
Classification
   ↓
Propagation
   ↓
Logging
   ↓
Response / Recovery
   ↓
Cleanup
   ↓
Exit Status

The combined M2 gap status is now:
GAP-001 — Graceful Shutdown       COMPLETE
GAP-002 — Connection Lifecycle    COMPLETE
GAP-003 — Error Handling          COMPLETE
The M2 server-side application baseline can therefore be considered complete pending the formal Phase 6 M2 completion review.