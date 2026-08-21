# PacketForge V1
# M1 Build System Documentation

---

# 1. Build System

PacketForge uses GNU Make and g++.

Compiler:

    g++

Language standard:

    C++20

Build command:

    make apps

---

# 2. Compiler Flags

Current compiler configuration:

    -std=c++20
    -Wall
    -Wextra
    -Wpedantic
    -g
    -O0
    -Iinclude
    -MMD
    -MP

Meaning:

    -std=c++20
        Use C++20.

    -Wall
        Enable common warnings.

    -Wextra
        Enable additional warnings.

    -Wpedantic
        Enable strict standard-related warnings.

    -g
        Generate debugging information.

    -O0
        Disable optimization during development.

    -Iinclude
        Search include/ for project headers.

    -MMD
        Generate dependency information.

    -MP
        Generate phony dependency targets.

---

# 3. Compilation

Compilation converts a .cpp file into an object file.

Example:

    src/network/socket.cpp

becomes:

    build/network/socket.o

The general Makefile rule is conceptually:

    build/network/%.o: src/network/%.cpp

This means:

    build/network/socket.o
    build/network/connection.o
    build/network/transport.o

can all be generated using the same pattern rule.

---

# 4. Linking

Linking combines object files into an executable.

Example:

    build/apps/server_main.o
    build/server/server.o
    build/network/socket.o
    build/network/connection.o
    ...

become:

    bin/packetforge_server

The client follows a similar process.

---

# 5. Why Directories Are Created During Build

The Makefile uses:

    mkdir -p $(dir $@)

before compiling or linking.

Example:

    mkdir -p build/network/

followed by:

    g++ ... -o build/network/socket.o

This is intentional.

The build directory may have been deleted using:

    make clean

Therefore the Makefile cannot assume that build/network/,
build/apps/, etc. already exist.

The mkdir command makes the build system self-contained and reproducible.

---

# 6. Why Existing Directories Are Not a Problem

The command:

    mkdir -p build/network/

does not mean:

    "Create a new directory even if one exists."

The -p option means:

    "Create the directory if necessary."

If the directory already exists, nothing is wrong.

Therefore repeated messages such as:

    mkdir -p build/network/

are normal.

---

# 7. Application Target

The applications target is:

    apps: $(SERVER_BIN) $(CLIENT_BIN)

This tells Make:

    make apps

must produce both:

    bin/packetforge_server
    bin/packetforge_client

The target then prints a confirmation message.

---

# 8. Clean Target

The clean target removes generated artifacts:

    rm -rf $(BUILD_DIR)/*
    rm -rf $(BIN_DIR)/*

It does not remove source code.

Therefore:

    make clean

is safe for returning the repository to a source-only state.

---

# 9. Incremental Builds

Make does not necessarily compile everything every time.

If:

    socket.cpp

has not changed and:

    build/network/socket.o

is newer than the source and its dependencies, Make can reuse the existing
object file.

If socket.cpp changes, socket.o is rebuilt.

This is one of the primary reasons for using Make rather than manually
running g++ commands.

---

# 10. Build Artifacts

Generated files include:

    *.o
    *.d
    executable binaries

These belong under:

    build/
    bin/

They should not normally be committed as source files.

---

# 11. Typical Development Cycle

Normal development:

    edit C++ source
           |
           v
       make apps
           |
           v
      run application

Example:

    make apps
    ./bin/packetforge_server

For a clean rebuild:

    make clean
    make apps

---

# 12. Important Distinction

The Makefile controls BUILDING.

C++ code controls BEHAVIOR.

For example:

    make apps

does not make the server listen on a port.

The C++ server implementation must do that.

Similarly:

    make apps

does not create a client/server connection.

The networking implementation must do that.

---

# 13. Current M1 Build Result

Successful output should result in:

    bin/packetforge_server
    bin/packetforge_client

Both executables have been verified to launch and return:

    exit code 0

This confirms the M1 build pipeline is operational.