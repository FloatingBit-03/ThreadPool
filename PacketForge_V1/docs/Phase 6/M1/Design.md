# PacketForge V1
# M1 Design Document

---

# 1. Design Goal

The design goal of M1 is to create a modular and maintainable foundation
for PacketForge.

The project is intentionally divided into logical components rather than
placing all C++ files in one directory.

The directory structure should communicate architectural boundaries.

---

# 2. Project Structure

Current conceptual structure:

    PacketForge_V1/
    |
    +-- apps/
    |   +-- client_main.cpp
    |   +-- server_main.cpp
    |
    +-- include/
    |   +-- common/
    |   +-- network/
    |   +-- protocol/
    |   +-- client/
    |   +-- server/
    |   +-- config/
    |
    +-- src/
    |   +-- common/
    |   +-- network/
    |   +-- protocol/
    |   +-- client/
    |   +-- server/
    |   +-- config/
    |
    +-- tests/
    |   +-- unit/
    |
    +-- build/
    |
    +-- bin/
    |
    +-- Makefile

---

# 3. Directory Responsibilities

## apps/

Contains application entry points.

Example:

    apps/server_main.cpp
    apps/client_main.cpp

These files contain the program entry point:

    int main()

They should primarily be responsible for starting the corresponding
application.

They should NOT contain the implementation of networking, protocol,
logging, connection management, etc.

---

# 4. include/

Contains public header files.

Headers describe interfaces used by other components.

Example:

    include/network/socket.h
    include/network/connection.h
    include/protocol/packet.h

The include directory is exposed to the compiler using:

    -Iinclude

This allows source files to include project headers consistently.

---

# 5. src/

Contains implementation files.

The src directory is divided according to logical subsystems.

---

## 5.1 common/

Shared infrastructure.

Examples:

    error.cpp
    logger.cpp
    buffer.cpp
    endian.cpp

Responsibilities may include:

- Error handling
- Logging
- Buffer utilities
- Byte-order conversion
- Shared low-level utilities

---

## 5.2 network/

Networking infrastructure.

Examples:

    socket.cpp
    connection.cpp
    transport.cpp

Responsibilities include:

- Socket abstraction
- Connection management
- Transport-level behavior

---

## 5.3 protocol/

Packet/protocol functionality.

Examples:

    packet.cpp
    encoder.cpp
    decoder.cpp

Responsibilities include:

- Packet representation
- Encoding
- Decoding
- Protocol-level serialization

---

## 5.4 client/

Client-side implementation.

Example:

    client.cpp

The client component should contain client-specific behavior rather than
application startup code.

---

## 5.5 server/

Server-side implementation.

Example:

    server.cpp

The server component should contain server-specific behavior rather than
application startup code.

---

## 5.6 config/

Configuration functionality.

Examples:

    config.cpp
    config_value.cpp
    config_loader.cpp

Configuration implementation exists as a separate subsystem so that future
configuration behavior does not become mixed into networking or application
startup logic.

---

# 6. tests/

Contains automated tests.

Current organization:

    tests/unit/

Tests are compiled separately from production application code.

Test executables are generated under:

    build/tests/

---

# 7. build/

The build directory contains generated compilation artifacts.

Examples:

    build/common/error.o
    build/network/socket.o
    build/protocol/packet.o
    build/apps/server_main.o

Dependency files may also be generated:

    build/common/error.d
    build/network/socket.d

The build directory is NOT source code.

It can be safely deleted and recreated by the build system.

---

# 8. bin/

Contains final executable applications.

Current binaries:

    bin/packetforge_server
    bin/packetforge_client

These are the final products of the application build process.

The bin directory contains executable artifacts, not source code.

---

# 9. Why .o Files Are Separated From .cpp Files

A C++ application normally follows:

    .cpp
      |
      | compilation
      v
    .o
      |
      | linking
      v
    executable

The .o file is an intermediate compilation artifact.

Keeping .o files inside build/ rather than beside .cpp files provides several
advantages:

1. Source directories remain clean.
2. Generated files are clearly separated.
3. Clean builds are easier.
4. Incremental builds are easier.
5. Different build configurations can later use different build directories.
6. Developers can immediately identify generated artifacts.

Therefore:

    src/network/socket.cpp

becomes:

    build/network/socket.o

rather than:

    src/network/socket.o

---

# 10. Why Not Put Everything in One Directory?

Technically, C++ allows this:

    PacketForge/
    |
    +-- client.cpp
    +-- server.cpp
    +-- socket.cpp
    +-- packet.cpp
    +-- logger.cpp
    +-- client_main.cpp
    +-- server_main.cpp
    +-- socket.o
    +-- packet.o
    +-- ...

However, this becomes difficult to maintain as the project grows.

PacketForge is intended to contain multiple logical subsystems.

Therefore the directory structure reflects the architecture.

For example:

    network/
        socket
        connection
        transport

is easier to understand than a flat directory containing dozens of
unrelated files.

---

# 11. Makefile Responsibility

The Makefile is responsible for describing the build graph.

It does not implement application behavior.

Its responsibilities are:

- Define compiler
- Define compiler flags
- Define directories
- Define source/object relationships
- Compile source files
- Link object files
- Build applications
- Build tests
- Track dependencies
- Clean generated artifacts

The Makefile answers:

    "How do I transform my source code into executable programs?"

It does NOT answer:

    "What should the server do?"

That belongs in C++ application code.

---

# 12. Build Graph

The server build can conceptually be represented as:

    server_main.cpp
           |
           v
    server_main.o
           |
           +---- server.o
           |
           +---- socket.o
           |
           +---- connection.o
           |
           +---- transport.o
           |
           +---- error.o
           |
           +---- logger.o
           |
           +---- buffer.o
           |
           +---- endian.o
           |
           v
    packetforge_server


The client build:

    client_main.cpp
           |
           v
    client_main.o
           |
           +---- client.o
           |
           +---- socket.o
           |
           +---- connection.o
           |
           +---- transport.o
           |
           +---- packet.o
           |
           +---- encoder.o
           |
           +---- decoder.o
           |
           +---- common objects
           |
           v
    packetforge_client

---

# 13. Dependency Tracking

The compiler flags include:

    -MMD
    -MP

These cause dependency information to be generated for object files.

The Makefile collects dependency files using:

    DEPS := $(shell find $(BUILD_DIR) -name "*.d" 2>/dev/null)

and includes them using:

    -include $(DEPS)

This allows Make to understand header dependencies.

For example:

    socket.cpp
        |
        +-- socket.h
        |
        +-- error.h

If socket.h changes, Make can determine that socket.o needs to be
recompiled.

---

# 14. Build Targets

The major targets are:

    make apps

Build client and server applications.

    make test

Build and run unit tests.

    make clean

Remove generated build artifacts and binaries.

---

# 15. M1 Runtime Behavior

The current application entry points are intentionally minimal.

Running:

    ./bin/packetforge_server

and:

    ./bin/packetforge_client

currently results in immediate termination.

An exit code of:

    0

indicates successful termination.

This does NOT imply that networking functionality is complete.

It only verifies that the executable can start and terminate successfully.

---

# 16. Architectural Principle

M1 establishes the following separation:

    Application
        |
        v
    Client / Server
        |
        v
    Network / Protocol
        |
        v
    Common Infrastructure

The application entry point should not directly contain all subsystem
implementation.

This separation will become increasingly important as PacketForge grows.