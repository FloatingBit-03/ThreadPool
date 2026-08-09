# PacketForge — Phase 3 Design Document

## 1. Overview

Phase 3 introduces the protocol packet layer of PacketForge.

The objective of this phase is to define a structured packet representation and provide mechanisms to convert packets between an in-memory representation and their wire-format byte representation.

Phase 3 consists of three primary modules:

* `Packet`
* `Encoder`
* `Decoder`

The existing `Endian` module from Phase 1 is reused by the `Encoder` and `Decoder` to ensure that multi-byte protocol fields are represented using network byte order.

The overall data flow is:
                In-Memory Packet
                       |
                       v
                  +---------+
                  | Encoder |
                  +---------+
                       |
                       | byte vector
                       v
                 Wire Format
                       |
                       v
                  +---------+
                  | Decoder |
                  +---------+
                       |
                       v
                In-Memory Packet

## 2. Phase 3 Objectives

The main objectives of Phase 3 are:

* Define the structure of a Packet.
* Provide packet metadata fields required by the protocol.
* Support arbitrary payload data.
* Encode a Packet into a byte buffer.
* Decode a byte buffer back into a Packet.
* Define a fixed-size packet header.
* Use network byte order for multi-byte fields.
* Validate packet data during decoding.
* Reject malformed packets.
* Integrate the existing Endian module.
* Provide unit tests for all three protocol modules.

# 3. Packet Module

## 3.1 Purpose

The `Packet` module represents a protocol packet in memory.

It provides storage and access to the fields required by the PacketForge protocol.

The Packet contains:

* Magic number
* Protocol version
* Flags
* Opcode
* Sequence ID
* Payload length
* Payload data

## 3.2 Packet Structure

The logical packet format is:
+----------------------+----------------+
| Field                | Size           |
+----------------------+----------------+
| Magic Number         | 4 bytes        |
| Version              | 1 byte         |
| Flags                | 1 byte         |
| Opcode               | 2 bytes        |
| Sequence ID          | 4 bytes        |
| Payload Length       | 4 bytes        |
| Payload              | Variable       |
+----------------------+----------------+

The fixed header is therefore:
4 + 1 + 1 + 2 + 4 + 4 = 16 bytes

Thus:
Packet::HEADER_SIZE = 16


The payload follows immediately after the header.

## 3.3 Packet Fields

### Magic Number

The magic number identifies a PacketForge packet.

It allows the decoder to determine whether the received byte stream represents a valid PacketForge packet.

The decoder rejects the packet when the magic number does not match the expected value.

### Version

The protocol version identifies the packet format version.

The default PacketForge version is:
1


### Flags

Flags provide space for protocol-level packet properties.

### Opcode

The opcode identifies the operation represented by the packet.

It is stored as a 16-bit value.

### Sequence ID

The sequence ID allows packets to be associated with a particular request or transaction.

It is stored as a 32-bit value.

### Payload Length

Payload length specifies the number of bytes contained in the payload.

It is stored as a 32-bit value.

### Payload

The payload contains variable-length application data.

The Packet class maintains the payload and derives its length from the payload size.

# 4. Encoder Module

## 4.1 Purpose

The `Encoder` converts a `Packet` object into its wire-format representation.

Its primary interface is:

std::vector<std::uint8_t>
encode(const Packet& packet) const;

The encoder is an object with a member `encode()` function.

Example:
Encoder encoder;

auto data = encoder.encode(packet);

## 4.2 Encoding Process

The encoder performs the following operations:

1. Create an output byte buffer.
2. Reserve enough space for the header and payload.
3. Convert multi-byte fields from host byte order to network byte order.
4. Append the packet header fields.
5. Append the payload.
6. Return the resulting byte vector.

The encoding flow is:
Packet
  |
  +-- Magic Number ----> hostToNetwork()
  |
  +-- Version
  |
  +-- Flags
  |
  +-- Opcode -----------> hostToNetwork()
  |
  +-- Sequence ID ------> hostToNetwork()
  |
  +-- Payload Length ---> hostToNetwork()
  |
  +-- Payload
  |
  v
Byte Vector

## 4.3 Endianness Handling

Multi-byte fields are converted using the existing Phase 1 `Endian` module.

The encoder uses:

common::Endian::hostToNetwork(...)
for:

* Magic number
* Opcode
* Sequence ID
* Payload length

The use of the Endian module ensures that the protocol wire format is independent of the host machine's native byte order.

The protocol therefore uses **network byte order (big-endian)** for multi-byte numeric fields.

# 5. Decoder Module

## 5.1 Purpose

The `Decoder` converts a wire-format byte vector into an in-memory `Packet`.

Its interface is:

static Packet decode(
    const std::vector<std::uint8_t>& data
);

Example:
auto decoded = Decoder::decode(data);

## 5.2 Decoding Process

The decoder performs the following operations:

1. Verify that the input contains at least a complete header.
2. Read the magic number.
3. Convert the magic number from network byte order to host byte order.
4. Validate the magic number.
5. Read the version.
6. Read the flags.
7. Read and convert the opcode.
8. Read and convert the sequence ID.
9. Read and convert the payload length.
10. Verify that the actual data size matches the declared payload length.
11. Extract the payload.
12. Construct and return the decoded Packet.

The decoding flow is:

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
Read Header Fields
    |
    v
networkToHost()
    |
    v
Validate Payload Length
    |
    v
Extract Payload
    |
    v
Packet

# 6. Packet Validation

The decoder performs validation before constructing the final packet.

## 6.1 Packet Size Validation

If the input data is smaller than the fixed header size, decoding fails.
data.size() < Packet::HEADER_SIZE

results in:
Invalid packet size

## 6.2 Magic Number Validation

The decoder converts the received magic number to host byte order and compares it with:

Packet::MagicNumber

If the values differ, decoding fails with:

Invalid magic number

## 6.3 Payload Length Validation

The decoder reads the payload length from the header.

The expected packet size is:
HEADER_SIZE + payload_length

The decoder requires:

data.size() == HEADER_SIZE + payload_length

If the values do not match, decoding fails with:

Payload size mismatch

This prevents malformed packets from being accepted.

# 7. Endian Module Integration

Phase 3 reuses the Endian module implemented during Phase 1.

The responsibilities are separated as follows:

+----------------+       +----------------+
|    Encoder     |       |    Decoder     |
+----------------+       +----------------+
        |                       |
        | hostToNetwork()       | networkToHost()
        |                       |
        +-----------+-----------+
                    |
                    v
             +-------------+
             |    Endian   |
             +-------------+

The Encoder converts:

Host representation
        |
        v
Network representation

The Decoder performs the reverse:

Network representation
        |
        v
Host representation

This keeps byte-order logic out of the Packet class and avoids duplicating endian conversion logic inside the protocol modules.

# 8. Module Responsibilities

| Module    | Responsibility                             |
| --------- | ------------------------------------------ |
| `Packet`  | Represents packet data in memory           |
| `Encoder` | Converts Packet to wire-format bytes       |
| `Decoder` | Converts wire-format bytes to Packet       |
| `Endian`  | Handles host/network byte-order conversion |

The modules therefore follow a clear separation of responsibilities.

# 9. Design Principles

Phase 3 follows these design principles:

### Separation of concerns

Packet storage, encoding, decoding, and byte-order conversion are implemented separately.

### Explicit wire format

The packet header has a clearly defined 16-byte layout.

### Validation at the boundary

The Decoder validates external byte data before accepting it as a Packet.

### Reuse

The existing Endian module is reused instead of implementing byte swapping separately in the Encoder and Decoder.

### Const correctness

Encoding does not modify the input Packet:

encode(const Packet& packet) const

### RAII and standard containers

Packet data and encoded data are represented using standard C++ containers such as:

std::vector<std::uint8_t>

# 10. Phase 3 Architecture

The final Phase 3 architecture can be summarized as:

                    PacketForge
                        |
                +-------+-------+
                |               |
            Protocol          Common
                |               |
        +-------+-------+    +--------+
        |       |       |    | Endian |
        |       |       |    +--------+
      Packet Encoder Decoder
        |       |       |
        +-------+-------+
                |
          Packet Format

The Protocol layer owns packet representation and serialization, while the Common layer provides reusable byte-order functionality.
