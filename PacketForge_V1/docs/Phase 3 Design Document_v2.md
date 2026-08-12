# PacketForge — Phase 3 Design Document

## Version 3 — Final / Frozen

# 1. Overview

Phase 3 introduces the protocol packet layer of PacketForge.

The objective of this phase is to define a structured packet representation and provide mechanisms to convert packets between an in-memory representation and their wire-format byte representation.

Phase 3 consists of three primary modules:

- `Packet`
- `Encoder`
- `Decoder`

The existing `Endian` module from Phase 1 is reused by the `Encoder` and `Decoder` to ensure that multi-byte protocol fields use network byte order.

The overall data flow is:

                In-Memory Packet
                       |
                       v
                +-------------+
                |   Encoder   |
                +-------------+
                       |
                       | byte vector
                       v
                   Wire Format
                       |
                       v
                +-------------+
                |   Decoder   |
                +-------------+
                       |
                       v
                In-Memory Packet

2. Objectives

The objectives of Phase 3 are:

Define the structure of a Packet.
Provide packet metadata fields required by the protocol.
Support arbitrary binary payload data.
Encode a Packet into a byte buffer.
Decode a byte buffer back into a Packet.
Define a fixed-size packet header.
Use network byte order for multi-byte fields.
Validate packet data during encoding and decoding.
Reject malformed packets.
Integrate the existing Endian module.
Provide unit tests for the protocol modules.
Establish a stable protocol API for higher-level modules.

All Phase 3 objectives have been implemented and verified.

3. Protocol Packet Format

PacketForge uses a fixed-size header followed by a variable-length payload.

+----------------------+----------------+
| Field                | Size           |
+----------------------+----------------+
| Magic Number         | 4 bytes        |
| Version              | 1 byte         |
| Flags                | 1 byte         |
| Opcode               | 2 bytes        |
| Sequence ID          | 4 bytes        |
| Payload Length       | 4 bytes        |
+----------------------+----------------+
| Payload              | Variable       |
+----------------------+----------------+

The fixed header size is:

4 + 1 + 1 + 2 + 4 + 4 = 16 bytes

Therefore:
Packet::HEADER_SIZE == 16

The payload immediately follows the 16-byte header.

4. Wire Format Layout

The exact byte offsets are:

Offset       Size       Field
----------------------------------------
0            4          Magic Number
4            1          Version
5            1          Flags
6            2          Opcode
8            4          Sequence ID
12           4          Payload Length
16           N          Payload

The total encoded packet size is:

Packet::HEADER_SIZE + payloadLength

or: 16 + payloadLength

5. Packet Module
5.1 Purpose

The Packet module represents a PacketForge protocol packet in memory.

It provides storage and access to packet metadata and payload data.

The Packet contains:

Magic number
Protocol version
Flags
Opcode
Sequence ID
Payload
Derived payload length

The Packet class does not perform serialization or network communication.

6. Packet Constants

The Packet class defines the following protocol constants:

static constexpr std::size_t HEADER_SIZE = 16;

static constexpr std::uint32_t MagicNumber = 0x50464B54;

static constexpr std::uint8_t VERSION = 1;

The magic number: 0x50464B54

represents: PFKT

and identifies PacketForge packets. The currently supported protocol version is:1

7. Packet Construction

A default Packet is initialized as:

Magic Number = Packet::MagicNumber
Version      = Packet::VERSION
Flags        = 0
Opcode       = 0
Sequence ID  = 0
Payload      = empty

Therefore, a newly constructed Packet represents a valid empty packet.

Example:
Packet packet;
results in:

Magic Number = 0x50464B54
Version      = 1
Flags        = 0
Opcode       = 0
Sequence ID  = 0
Payload      = {}

8. Packet Fields
8.1 Magic Number

The magic number identifies a PacketForge packet.

It is stored internally as: std::uint32_t

The decoder validates that the received value matches: Packet::MagicNumber

An incorrect magic number causes decoding to fail.

8.2 Version

The protocol version identifies the packet format version.

The currently supported version is:

Packet::VERSION

which equals: 1

The version is stored as: std::uint8_t

A packet containing an unsupported version is considered invalid.

8.3 Flags

Flags provide space for protocol-level packet properties.

Flags are stored as: std::uint8_t

The current implementation does not assign specific meanings to individual flag bits.

8.4 Opcode

The opcode identifies the operation represented by the packet.

It is stored as: std::uint16_t

The opcode is encoded in network byte order.

8.5 Sequence ID

The sequence ID allows packets to be associated with a request or transaction.

It is stored as: std::uint32_t

The sequence ID is encoded in network byte order.

8.6 Payload Length

The payload length specifies the number of bytes contained in the payload.

The Packet class derives this value from: payload_.size()

through: std::uint32_t payloadLength() const noexcept;

There is intentionally no independent payload-length setter.

This prevents the Packet object from containing a payload and payload length that disagree.

8.7 Payload

The payload contains arbitrary binary application data.

It is stored as: std::vector<std::uint8_t>

The payload may be empty.

The payload is placed immediately after the fixed 16-byte header during encoding.

9. Packet Validation

The Packet class provides: bool isValid() const noexcept;

The validation rules are:

Magic number must equal Packet::MagicNumber
Version must equal Packet::VERSION

Therefore:

Packet packet;

EXPECT_TRUE(packet.isValid());

while:

Packet packet;

packet.setVersion(2);

EXPECT_FALSE(packet.isValid());

The validation logic ensures that unsupported protocol versions are not treated as valid packets.

10. Encoder Module
10.1 Purpose

The Encoder converts a Packet object into its wire-format byte representation.

Its public interface is: std::vector<std::uint8_t>
encode(const Packet& packet) const;

Example:

Encoder encoder;

auto data = encoder.encode(packet);

The Encoder does not modify the source Packet.

11. Encoder Responsibilities

The Encoder is responsible for:

Validating the Packet.
Creating the output byte buffer.
Reserving sufficient capacity.
Converting multi-byte values to network byte order.
Writing the fixed header.
Appending the payload.
Returning the encoded byte vector.

12. Encoder Validation

The Encoder validates the Packet before serialization.

If: packet.isValid() == false

the Encoder rejects the Packet.

The current implementation throws: std::invalid_argument

For example:

Packet packet;

packet.setVersion(2);

EXPECT_FALSE(packet.isValid());

EXPECT_THROW(
    encoder.encode(packet),
    std::invalid_argument
);

This prevents invalid protocol packets from being serialized.

13. Encoding Process

The Encoder follows this process:

Packet
  |
  v
Validate Packet
  |
  v
Create Output Buffer
  |
  v
Reserve Header + Payload
  |
  v
Encode Magic Number
  |
  v
Encode Version
  |
  v
Encode Flags
  |
  v
Encode Opcode
  |
  v
Encode Sequence ID
  |
  v
Encode Payload Length
  |
  v
Append Payload
  |
  v
Byte Vector

14. Encoder Endianness

Multi-byte fields are converted using: common::Endian::hostToNetwork(...)

The Encoder applies this conversion to:

Magic number
Opcode
Sequence ID
Payload length

The following fields are single-byte values and require no endian conversion:

Version
Flags

The wire format therefore uses network byte order, which is big-endian, for all multi-byte numeric fields.

15. Decoder Module
15.1 Purpose

The Decoder converts a wire-format byte vector back into an in-memory Packet.

Its public interface is:

static Packet decode(
    const std::vector<std::uint8_t>& data
);

Example:

auto packet = Decoder::decode(data);

The Decoder is stateless and therefore exposes decoding through a static function.

16. Decoder Responsibilities

The Decoder is responsible for:

Validating minimum packet size.
Reading the packet header.
Converting network byte order to host byte order.
Validating the magic number.
Reading the protocol version.
Reading flags.
Reading opcode.
Reading sequence ID.
Reading payload length.
Validating packet size.
Extracting the payload.
Constructing the Packet.
Rejecting malformed wire data.

17. Decoder Process

The decoding process is:

Byte Vector
     |
     v
Validate Header Size
     |
     v
Read Magic Number
     |
     v
networkToHost()
     |
     v
Validate Magic Number
     |
     v
Read Version
     |
     v
Read Flags
     |
     v
Read Opcode
     |
     v
networkToHost()
     |
     v
Read Sequence ID
     |
     v
networkToHost()
     |
     v
Read Payload Length
     |
     v
networkToHost()
     |
     v
Validate Packet Size
     |
     v
Extract Payload
     |
     v
Construct Packet
     |
     v
Return Packet

18. Decoder Header Validation

The Decoder first checks whether the received data contains the complete fixed header.

The following condition is invalid: data.size() < Packet::HEADER_SIZE

The Decoder rejects such input with: Invalid packet size

This prevents out-of-bounds access while reading the header.

19. Decoder Magic Number Validation

The Decoder reads the first four bytes.

The value is converted from network byte order using: common::Endian::networkToHost(...)

The resulting value must equal: Packet::MagicNumber

Otherwise the Decoder rejects the packet with: Invalid magic number

20. Decoder Payload Validation

The Decoder reads the payload length from offset 12.

The expected packet size is:Packet::HEADER_SIZE + payloadLength

The actual input must satisfy:

data.size() ==
    Packet::HEADER_SIZE + payloadLength

If the values differ, decoding fails with: Payload size mismatch

This prevents:

Truncated packets.
Extra trailing data.
Incorrect payload lengths.
Malformed packet data.

21. Decoder Field Offsets

The Decoder uses the following protocol offsets:

Offset       Field
----------------------------
0            Magic Number
4            Version
5            Flags
6            Opcode
8            Sequence ID
12           Payload Length
16           Payload

The multi-byte fields are decoded using: common::Endian::networkToHost(...)

22. Decoder Exceptions

Malformed input is rejected using exceptions.

Current error conditions include:

Invalid packet size
Invalid magic number
Payload size mismatch

These are currently represented using: std::runtime_error

The Decoder therefore never silently accepts malformed wire data.

23. Endian Module Integration

Phase 3 reuses the Endian module implemented during Phase 1.

The relationship is:

+-------------------+       +-------------------+
|      Encoder      |       |      Decoder      |
+-------------------+       +-------------------+
| hostToNetwork()   |       | networkToHost()   |
+---------+---------+       +---------+---------+
          |                           |
          +-------------+-------------+
                        |
                        v
                 +-------------+
                 |    Endian   |
                 +-------------+

The Encoder performs:

Host Representation
        |
        v
Network Representation

The Decoder performs:

Network Representation
        |
        v
Host Representation

This keeps endian conversion logic outside the Packet class.

24. Module Responsibilities
Module	Responsibility
Packet	Represents packet data in memory
Encoder	Converts Packet objects to wire-format bytes
Decoder	Converts wire-format bytes to Packet objects
Endian	Handles host/network byte-order conversion

Each module has a distinct responsibility.

25. Error Handling Boundary

Phase 3 uses exceptions for protocol serialization and deserialization failures.

The Encoder uses: std::invalid_argument

for invalid Packet objects.

The Decoder uses: std::runtime_error

for malformed wire-format data.

The existing: common::Error

continues to serve the lower-level Common and Network layers.

Phase 3 does not introduce common::Error into the Encoder/Decoder API.

This keeps the protocol interface simple and independent from network resource management.

26. Design Principles
26.1 Separation of Concerns

Packet representation, serialization, deserialization, and endian conversion are separate responsibilities.

Packet
  |
  +-- Data representation

Encoder
  |
  +-- Serialization

Decoder
  |
  +-- Deserialization

Endian
  |
  +-- Byte-order conversion
26.2 Explicit Wire Format

The protocol defines a deterministic 16-byte header followed by a variable-length payload.

26.3 Validation at the Boundary

Both directions of the protocol boundary perform validation.

Packet -> Encoder
        |
        +-- Validate Packet

Bytes -> Decoder
        |
        +-- Validate Wire Data
26.4 Reuse

The existing Endian module is reused instead of duplicating byte-order conversion logic.

26.5 Const Correctness

Encoding does not modify the source Packet: encode(const Packet& packet) const;

Payload access is provided through a const reference:

const std::vector<std::uint8_t>&
payload() const noexcept;

26.6 RAII and Standard Containers

Packet payloads and encoded data use: std::vector<std::uint8_t>

No manual memory management is required for packet data.

27. Phase 3 Architecture

The final Phase 3 architecture is:

                    PacketForge
                        |
              +---------+---------+
              |                   |
          Protocol              Common
              |                   |
      +-------+-------+       +--------+
      |       |       |       | Endian |
      |       |       |       +--------+
    Packet Encoder Decoder
      |       |       |
      +-------+-------+
              |
        Packet Format

The Protocol layer owns:

Packet representation.
Serialization.
Deserialization.
Protocol validation.

The Common layer provides reusable infrastructure such as:

Endian conversion.
Error handling.
Buffer.
Logger.

28. Relationship to Network Layer

The Protocol layer does not directly manage sockets or connections.

The architecture is:

Application
     |
     v
Client / Server
     |
     v
Protocol
     |
     +-- Packet
     +-- Encoder
     +-- Decoder
     |
     v
Network
     |
     +-- Transport
     +-- Connection
     +-- Socket

Protocol modules operate on memory and byte vectors.

Network modules are responsible for moving those bytes.

This separation allows the Client and Server layers to use the protocol without implementing serialization or socket management themselves.

29. Phase 3 API Boundary

The stable protocol API consists of:

Packet
Encoder
Decoder

Higher-level modules should use these interfaces rather than directly manipulating the packet wire format.

For example, the Client layer should conceptually perform:

Create Packet
     |
     v
Encode Packet
     |
     v
Byte Vector
     |
     v
Transport

and:

Transport
     |
     v
Byte Vector
     |
     v
Decode Packet
     |
     v
Packet

30. Lessons Established by Phase 3

Phase 3 establishes several architectural rules for subsequent development.

Rule 1 — Protocol logic stays in Protocol

Client and Server modules should not duplicate:

Packet construction logic.
Serialization logic.
Deserialization logic.
Endian conversion.
Rule 2 — Network logic stays in Network

Protocol modules should not directly manage:

Sockets.
Connections.
Transport operations.
Rule 3 — Validate at boundaries

External wire data must be validated before being converted into internal protocol objects.

Invalid internal packets must also be rejected before serialization.

Rule 4 — Higher layers consume lower layers

The Client should use: Network + Protocol

rather than reimplementing either layer.

31. Next Phase

With Phase 3 frozen, development proceeds to:

Phase 4 — Client Module

The Phase 4 process will be:

1. Design Client interface

          |
          v

2. Compare Client API against
   existing architecture

          |
          v

3. Verify compatibility with:
   - Socket
   - Connection
   - Transport
   - Packet
   - Encoder
   - Decoder
   - common::Error

          |
          v

4. Freeze Client API

          |
          v

5. Implement client.cpp

          |
          v

6. Write and run Client tests

          |
          v

7. Update Phase 4 documentation

The Phase 4 Client should consume the frozen Phase 3 protocol layer rather than modify it.

32. Final Phase 3 Summary

Phase 3 establishes the PacketForge binary protocol layer.

It provides:

Packet
   |
   v
Encoder
   |
   v
Wire Format
   |
   v
Decoder
   |
   v
Packet

with:

A fixed 16-byte packet header.
PacketForge magic number 0x50464B54.
Protocol version 1.
Packet flags.
16-bit opcode.
32-bit sequence ID.
32-bit payload length.
Variable-length binary payload.
Network byte order for multi-byte fields.
Packet validation.
Encoder validation.
Decoder validation.
Endian module integration.
Unit-test coverage.
Stable interfaces for higher-level modules.