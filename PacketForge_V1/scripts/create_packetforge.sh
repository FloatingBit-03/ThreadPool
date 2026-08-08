#!/bin/bash

set -e

# ------------------------------------------------------------------
# Determine project root (parent of scripts directory)
# ------------------------------------------------------------------

PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

echo "Project root: $PROJECT_ROOT"
echo "Creating PacketForge project structure..."

# ------------------------------------------------------------------
# Create directories
# ------------------------------------------------------------------

mkdir -p "$PROJECT_ROOT"/{include,src,apps,tests,build,bin,docs}

mkdir -p "$PROJECT_ROOT/include"/{client,server,network,protocol,common}
mkdir -p "$PROJECT_ROOT/src"/{client,server,network,protocol,common}

mkdir -p "$PROJECT_ROOT/tests"/{unit,integration,data}

# ------------------------------------------------------------------
# Header files
# ------------------------------------------------------------------

touch "$PROJECT_ROOT/include/client/client.hpp"
touch "$PROJECT_ROOT/include/server/server.hpp"

touch "$PROJECT_ROOT/include/network/socket.hpp"

touch "$PROJECT_ROOT/include/protocol/packet.hpp"
touch "$PROJECT_ROOT/include/protocol/encoder.hpp"
touch "$PROJECT_ROOT/include/protocol/decoder.hpp"

touch "$PROJECT_ROOT/include/common/buffer.hpp"
touch "$PROJECT_ROOT/include/common/logger.hpp"
touch "$PROJECT_ROOT/include/common/endian.hpp"
touch "$PROJECT_ROOT/include/common/error.hpp"

# ------------------------------------------------------------------
# Source files
# ------------------------------------------------------------------

touch "$PROJECT_ROOT/src/client/client.cpp"
touch "$PROJECT_ROOT/src/server/server.cpp"

touch "$PROJECT_ROOT/src/network/socket.cpp"

touch "$PROJECT_ROOT/src/protocol/packet.cpp"
touch "$PROJECT_ROOT/src/protocol/encoder.cpp"
touch "$PROJECT_ROOT/src/protocol/decoder.cpp"

touch "$PROJECT_ROOT/src/common/buffer.cpp"
touch "$PROJECT_ROOT/src/common/logger.cpp"
touch "$PROJECT_ROOT/src/common/endian.cpp"
touch "$PROJECT_ROOT/src/common/error.cpp"

# ------------------------------------------------------------------
# Application entry points
# ------------------------------------------------------------------

touch "$PROJECT_ROOT/apps/client_main.cpp"
touch "$PROJECT_ROOT/apps/server_main.cpp"

# ------------------------------------------------------------------
# Test files
# ------------------------------------------------------------------

touch "$PROJECT_ROOT/tests/unit/test_packet.cpp"
touch "$PROJECT_ROOT/tests/unit/test_socket.cpp"

touch "$PROJECT_ROOT/tests/integration/test_client_server.cpp"

# ------------------------------------------------------------------
# Root files
# ------------------------------------------------------------------

touch "$PROJECT_ROOT/Makefile"
touch "$PROJECT_ROOT/README.md"
touch "$PROJECT_ROOT/.gitignore"

# ------------------------------------------------------------------
# Done
# ------------------------------------------------------------------

echo
echo "Project structure created successfully."
echo

if command -v tree >/dev/null 2>&1; then
    tree "$PROJECT_ROOT"
else
    find "$PROJECT_ROOT" | sort
fi

echo
echo "Done."