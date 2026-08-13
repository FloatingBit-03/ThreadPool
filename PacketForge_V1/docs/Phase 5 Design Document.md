# PacketForge — Phase 5 Server Design

## 1. Purpose
Phase 5 introduces the `Server` abstraction.

The purpose of the Server layer is to provide a simple and controlled
interface for creating a listening TCP server without exposing the
underlying socket-management details to higher layers.

The Server is responsible for:

- Creating the listening socket
- Binding the socket to an address and port
- Listening for incoming connections
- Tracking server lifecycle state
- Reporting the actual bound port
- Accepting incoming client connections
- Releasing the listening socket
- Supporting move semantics

The Server does not own client application logic or packet-processing
logic.

## 2. Server API

The public API is defined in: include/server/server.hpp

The current interface is:

class Server
{
public:

    Server();
    ~Server();

    Server(const Server&) = delete;
    Server& operator=(const Server&) = delete;

    Server(Server&& other) noexcept;
    Server& operator=(Server&& other) noexcept;

    common::Error start(
        const std::string& address,
        std::uint16_t port
    );

    void stop() noexcept;

    bool isRunning() const noexcept;

    std::uint16_t port() const noexcept;

    common::Error accept(
        network::Connection& connection
    );

private:

    network::Socket socket_;

    bool running_;

    std::uint16_t port_;
};

3. Responsibilities

3.1 Socket Creation
Server::start() creates the underlying network::Socket.

The Server does not directly manage the native socket descriptor.
Instead, it delegates socket operations to the existing Socket
abstraction.

3.2 Binding
The server binds the listening socket to the address and port supplied to start().

For example:
server.start(
    "127.0.0.1",
    8080
);

The Server therefore acts as the lifecycle owner of the listening socket while Socket remains responsible for the low-level socket
operation.

3.3 Listening
After successful binding, the Server places the socket into listening mode.

The listening backlog is delegated to the operating system using: SOMAXCONN

The Server does not expose backlog configuration in the current API.This keeps the Phase 5 API intentionally small.

3.4 Ephemeral Ports
The Server supports:

server.start(
    "127.0.0.1",
    0
);

Port 0 instructs the operating system to select an available ephemeral port.
After the socket is successfully bound, the Server retrieves the actual assigned port using getsockname().

The value is stored in: port_;
and exposed through: server.port();

This is important for tests and for future dynamically allocated server instances.

4. Server Lifecycle

The Server has two logical states:

                 start()
    STOPPED --------------------> RUNNING
       ^                            |
       |                            |
       +----------- stop() --------+
Initial state

A newly constructed Server has:

running_ == false
port_ == 0

The listening socket is closed.

Running state

After successful start():

running_ == true
port_ != 0
socket_.isOpen() == true
Stopped state

After stop():

running_ == false
port_ == 0
socket_.isOpen() == false

5. Start Failure Handling
start() can fail at several stages:

create()
   |
   +-- failure --> return error
   |
   v
bind()
   |
   +-- failure --> close socket --> return error
   |
   v
listen()
   |
   +-- failure --> close socket --> return error
   |
   v
getsockname()
   |
   +-- failure --> close socket --> return error
   |
   v
RUNNING

A failed start must not leave the Server in a partially running state.

Therefore, when binding, listening, or retrieving the assigned port
fails, the socket is closed before returning the error.

6. Starting an Already Running Server
Calling start() while the Server is already running is rejected.
The Server does not attempt to replace or recreate the existing
listening socket.

This prevents accidental resource replacement and makes the lifecycle explicit.

7. Stop Semantics
stop() is intentionally idempotent.
Calling: server.stop();

when the Server is already stopped is safe. Calling it multiple times must not:
Crash
Double-close the socket
Leave running_ set
Preserve an obsolete port number

After every call to stop():
running_ == false
port_ == 0
socket_.isOpen() == false

8. Connection Acceptance
The Server accepts client connections through:
common::Error accept(
    network::Connection& connection
);

The Server itself owns the listening socket. The accepted client socket is transferred into the supplied Connection.

Conceptually:
Server
  |
  | listening socket
  |
  v
accept()
  |
  | new client socket
  v
Connection

The Server remains responsible for the listening socket after a client is accepted.

Therefore:
Server running
       |
       +---- accept client A
       |
       +---- accept client B
       |
       +---- accept client C

The Server does not stop after accepting one connection.

9. Accept While Stopped
Calling accept() while the Server is not running is rejected. No attempt is made to call the underlying socket accept operation.
This provides an explicit lifecycle boundary:
STOPPED
   |
   +-- accept() --> error

10. Ownership Model
The Server owns its listening socket: network::Socket socket_;
The Server does not own the Connection passed to: accept(connection);

The caller owns the Connection object.This produces the following ownership model:
Server
  |
  └── owns listening Socket

Caller
  |
  └── owns Connection
           |
           └── owns accepted client Socket

11. Copy and Move Semantics
A Server cannot be copied:
Server(const Server&) = delete;
Server& operator=(const Server&) = delete;

This prevents accidental duplication of socket ownership.

The Server is movable:
Server(Server&& other) noexcept;
Server& operator=(Server&& other) noexcept;

Moving a Server transfers:
Listening socket ownership
Running state
Bound port

The moved-from Server becomes inactive:
other.running_ = false;
other.port_ = 0;

The moved-to Server becomes the owner of the listening socket.

12. Resource Management
The Server follows RAII principles. Its destructor calls: stop();

Therefore, a Server that goes out of scope releases its listening socket automatically.

This prevents the caller from having to manually close the listening socket.

13. Error Handling
The Server uses: common::Error for operational failures.
The Server does not throw exceptions for normal socket failures.

Examples include:
Invalid address
Invalid port
Socket creation failure
Bind failure
Listen failure
Accept failure
Starting an already-running server
Accepting while stopped

14. Layering
The Server depends on the existing networking abstractions:

Server
  |
  +--> Connection
  |
  +--> Socket
  |
  +--> Error

The Server does not directly implement:

Packet encoding
Packet decoding
Transport protocol
Client application logic
Packet dispatching
Thread management

Those responsibilities belong to higher layers or future phases.

15. Phase 5 Scope
Phase 5 intentionally provides a synchronous server abstraction.

The following are outside the current scope:

Event loops
Worker threads
Thread pools
Multiple-client management
Asynchronous accept
Connection registries
Packet dispatch
Client session management
Graceful connection shutdown policies
Server configuration objects

These can be introduced in later phases without changing the basic
Server ownership model.