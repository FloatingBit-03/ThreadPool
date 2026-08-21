Phase 6 — Milestones
M1 — Application Architecture

Define the final application layer:

apps/
├── server_main.cpp
└── client_main.cpp

Decide how applications configure and use the existing Server and Client APIs.

M2 — Server Application

Build the first real executable around:

Server
 ├── Socket
 └── Connection

Responsibilities:

Configure address/port
Start server
Accept clients
Maintain server lifecycle
Handle shutdown
M3 — Client Application

Build the client executable around:

Client
 ├── Connection
 └── Transport
      └── Protocol

Responsibilities:

Configure server address
Connect
Create/send packets
Receive responses
Disconnect cleanly
M4 — Client ↔ Server Integration

This is the core milestone of Phase 6.

Establish the complete path:

Client
   │
   │ TCP
   ▼
Server
   │
   ▼
Connection
   │
   ▼
Transport
   │
   ▼
Decoder
   │
   ▼
Packet
   │
   ▼
Application Logic
   │
   ▼
Packet
   │
   ▼
Encoder
   │
   ▼
Transport
   │
   ▼
Client

At this point we prove that the modules developed independently in Phases 1–5 actually work together.

M5 — Request/Response Workflow

Define one complete application-level exchange.

For example:

Client
   │
   │ Request Packet
   ▼
Server
   │
   │ Decode
   ▼
Request Handler
   │
   │ Process
   ▼
Response Packet
   │
   │ Encode
   ▼
Client

Start with one simple deterministic request/response, rather than introducing complicated business logic.

M6 — Integration Test Suite

Create:

tests/integration/

Test the complete system rather than individual classes.

Minimum scenarios:

Server starts
Client connects
Client sends request
Server receives request
Server decodes packet
Server generates response
Client receives response
Client decodes response
Connection closes cleanly
Invalid request handling
Server shutdown behavior
M7 — Failure & Lifecycle Validation

Validate real-world failure paths:

Client starts before server
Server unavailable
Connection failure
Malformed packet
Client disconnects unexpectedly
Server stops while client is connected
Repeated connection attempts
Repeated shutdown

The goal is to ensure that integration does not introduce resource leaks or inconsistent states.

M8 — Final Build & Test Pipeline

Update the Makefile so the project can build and test everything consistently:

make
make test
make integration-test
make clean

Ideally:

make test
     │
     ├── Common tests
     ├── Network tests
     ├── Protocol tests
     ├── Client tests
     ├── Server tests
     └── Integration tests
M9 — Final Documentation

Document:

Final architecture
Application API
Client/server workflow
Packet flow
Build instructions
Test instructions
Integration test strategy
Known limitations
Future enhancements

This becomes the project's v1 documentation baseline.

M10 — Project Completion Review

Perform the final review:

Phase 1  Common       ✓
Phase 2  Network      ✓
Phase 3  Protocol     ✓
Phase 4  Client       ✓
Phase 5  Server       ✓
Phase 6  Integration  → FINAL

Verify:

All unit tests pass
Integration tests pass
Client and server communicate
Resources are correctly released
Public APIs are stable
Build is reproducible
Documentation matches implementation
Final Phase 6 Definition

The simplest way to remember the route is:

        PHASE 1
         Common
            │
            ▼
        PHASE 2
         Network
            │
            ▼
        PHASE 3
        Protocol
            │
            ▼
        PHASE 4
         Client
            │
            ▼
        PHASE 5
         Server
            │
            ▼
        ┌──────────────┐
        │   PHASE 6    │
        │ APPLICATION + │
        │ INTEGRATION  │
        └──────────────┘
            │
            ▼
     Complete PacketForge
          Core Project