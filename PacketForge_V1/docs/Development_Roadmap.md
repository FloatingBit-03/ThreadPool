# PacketForge Development Roadmap

## Overview

This document defines the recommended development sequence for implementing PacketForge module by module.
The goal is to build the system incrementally, where each module provides a stable foundation for the modules above it.

The dependency flow is:

```
Common Utilities
        |
        v
Network Layer
        |
        v
Protocol Layer
        |
        v
Client / Server Layer
        |
        v
Application Layer
        |
        v
Testing and Optimization
```

Each module should be completed and tested before moving to the next layer.

---

# Phase 1: Project Foundation

## Goal

Prepare the project infrastructure before implementing networking functionality.

## Tasks

### Build System

Implement:

* Makefile
* Compiler flags
* Include paths
* Debug and release builds
* Dependency tracking

Expected output:

```
bin/
├── packetforge_client
└── packetforge_server
```

---

### Coding Standards

Define:

* C++ standard version
* Naming conventions
* Header organization
* Error handling strategy
* Logging strategy

Recommended:

```
C++20
-Wall
-Wextra
-Werror
```

---

# Phase 2: Common Module

Location:

```
include/common/
src/common/
```

The common module contains reusable utilities used by all other modules.

---

## 2.1 Error Handling

Files:

```
include/common/error.hpp
src/common/error.cpp
```

Responsibilities:

* Error codes
* Exception types
* Error messages
* Result handling

Example:

```
NetworkError
ProtocolError
ConnectionError
```

Completion criteria:

* All modules can report errors consistently.

---

## 2.2 Logger

Files:

```
include/common/logger.hpp
src/common/logger.cpp
```

Responsibilities:

* Info messages
* Warning messages
* Error messages
* Debug output

Example:

```
[INFO] Server started
[ERROR] Connection failed
```

Completion criteria:

* Replace direct std::cout/std::cerr usage.

---

## 2.3 Buffer Management

Files:

```
include/common/buffer.hpp
src/common/buffer.cpp
```

Responsibilities:

* Byte storage
* Reading/writing bytes
* Cursor management

Example:

```
Buffer
 |
 |-- write(uint32_t)
 |-- write(string)
 |-- read(uint32_t)
```

This module becomes the foundation for packet serialization.

---

## 2.4 Endian Utilities

Files:

```
include/common/endian.hpp
src/common/endian.cpp
```

Responsibilities:

* Host-to-network conversion
* Network-to-host conversion
* Big endian handling

Example:

```
uint32_t hostToNetwork()
uint32_t networkToHost()
```

---

# Phase 3: Network Module

Location:

```
include/network/
src/network/
```

This module abstracts operating system socket APIs.

---

## 3.1 Socket Class

Files:

```
socket.hpp
socket.cpp
```

Responsibilities:

* Create socket
* Close socket
* Manage file descriptor
* Send bytes
* Receive bytes

Design:

Use RAII.

Example:

```cpp
Socket socket;
```

The destructor automatically releases resources.

---

## 3.2 TCP Client Socket Support

Responsibilities:

* Create TCP socket
* Connect to server
* Send data
* Receive data

Example:

```
Client
 |
 Socket
 |
 TCP Connection
```

---

## 3.3 TCP Server Socket Support

Responsibilities:

* Bind address
* Listen
* Accept connections

Example:

```
Server Socket
       |
       |
   Client Socket
```

---

# Phase 4: Protocol Module

Location:

```
include/protocol/
src/protocol/
```

This module defines PacketForge communication rules.

---

## 4.1 Packet Design

Files:

```
packet.hpp
packet.cpp
```

Define packet structure.

Example:

```
+----------------+
| Header         |
+----------------+
| Opcode         |
| Length         |
| Sequence ID    |
+----------------+
| Payload        |
+----------------+
```

Responsibilities:

* Packet creation
* Packet validation
* Packet metadata

---

## 4.2 Encoder

Files:

```
encoder.hpp
encoder.cpp
```

Responsibilities:

Convert objects into binary data.

Example:

```
Packet Object
      |
      v
Binary Stream
```

---

## 4.3 Decoder

Files:

```
decoder.hpp
decoder.cpp
```

Responsibilities:

Convert binary data back into objects.

Example:

```
Binary Stream
      |
      v
Packet Object
```

---

# Phase 5: Client Module

Location:

```
include/client/
src/client/
```

Responsibilities:

* Connect to server
* Create packets
* Send requests
* Receive responses
* Process server replies

Example:

```
Client
 |
 TcpSocket
 |
 Encoder
 |
 Packet
```

Implementation:

```
client.hpp
client.cpp
```

---

# Phase 6: Server Module

Location:

```
include/server/
src/server/
```

Responsibilities:

* Start listening
* Accept clients
* Receive packets
* Decode requests
* Process commands
* Send responses

Example:

```
Server
 |
 Socket
 |
 Decoder
 |
 Packet Handler
```

Implementation:

```
server.hpp
server.cpp
```

---

# Phase 7: Application Layer

Location:

```
apps/
```

Files:

```
client_main.cpp
server_main.cpp
```

Purpose:

Create executable programs.

Example:

Client:

```cpp
int main()
{
    Client client;
    client.connect();
}
```

Server:

```cpp
int main()
{
    Server server;
    server.start();
}
```

The application layer should contain minimal logic.

---

# Phase 8: Testing Strategy

Location:

```
tests/
```

---

## Unit Tests

Test individual modules.

Examples:

```
test_buffer.cpp
test_packet.cpp
test_encoder.cpp
test_decoder.cpp
test_socket.cpp
```

---

## Integration Tests

Test complete workflows.

Examples:

```
Client connects to Server

Client sends Packet

Server processes Packet

Server sends Response
```

---

# Phase 9: Advanced Features

After the basic system works, add advanced capabilities.

## Networking Improvements

* Non-blocking sockets
* Event loop
* epoll support
* Multiple client handling

---

## Protocol Improvements

* Packet versioning
* Compression
* Encryption
* Authentication

---

## Performance Improvements

* Memory pooling
* Zero-copy buffers
* Thread pool
* Connection pooling

---

# Recommended Implementation Order

Follow this exact sequence:

```
1. Build System
        |
2. Common Utilities
        |
3. Socket Abstraction
        |
4. TCP Client
        |
5. TCP Server
        |
6. Buffer System
        |
7. Packet Format
        |
8. Encoder / Decoder
        |
9. Client Class
        |
10. Server Class
        |
11. Applications
        |
12. Tests
        |
13. Optimization
```

---

# Final Goal

At completion, PacketForge should provide:

```
packetforge_client
        |
        |
   PacketForge Protocol
        |
        |
packetforge_server
```

with:

* Reliable TCP communication
* Custom packet format
* Serialization/deserialization
* Clean modular architecture
* Unit and integration testing
* Extensible protocol design

This roadmap ensures that each layer is stable before dependent layers are introduced, reducing complexity and making the project easier to debug, extend, and maintain.
