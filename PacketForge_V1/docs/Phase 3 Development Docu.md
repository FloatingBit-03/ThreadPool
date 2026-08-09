# PacketForge — Phase 3 Development Document

## 1. Overview
Phase 3 implements the PacketForge protocol packet serialization layer.

The implementation consists of three protocol modules:

1. `Packet`
2. `Encoder`
3. `Decoder`

The implementation also integrates the previously developed `Endian` module from Phase 1.

The development process focused on implementing the packet representation first, followed by serialization and deserialization, and finally validating the complete protocol flow through unit tests.

# 2. Source Files

The primary Phase 3 source files are:

include/protocol/packet.hpp
src/protocol/packet.cpp

include/protocol/encoder.hpp
src/protocol/encoder.cpp

include/protocol/decoder.hpp
src/protocol/decoder.cpp

The Phase 1 Endian module is reused:

include/common/endian.hpp
src/common/endian.cpp

Unit tests are located under:

tests/unit/


with the following Phase 3 test files:

tests/unit/test_packet.cpp
tests/unit/test_encoder.cpp
tests/unit/test_decoder.cpp

# 3. Packet Module Development

The Packet class was implemented as the central in-memory representation of the protocol packet.

The class provides accessors and setters for:

* Magic number
* Version
* Flags
* Opcode
* Sequence ID
* Payload

The payload length is derived from the stored payload.

This avoids maintaining a separate mutable payload-length field that could become inconsistent with the actual payload.

# 4. Encoder Development

The Encoder was implemented as a class with a member function:

Encoder::encode(const Packet& packet) const

An `Encoder` object is created before encoding:

Encoder encoder;

auto data = encoder.encode(packet);

The encoder constructs a `std::vector<std::uint8_t>` containing the complete wire representation.

## 4.1 Header Serialization

The encoder writes the fields in protocol order:

Magic Number
Version
Flags
Opcode
Sequence ID
Payload Length
Payload

The resulting header is 16 bytes.
## 4.2 Network Byte Order

During implementation, an important issue was identified with the representation of the magic number and other multi-byte fields.

The existing Endian module was therefore integrated into the Encoder.

For example:
common::Endian::hostToNetwork(
    packet.magicNumber()
)

The same conversion is applied to:

Magic Number
Opcode
Sequence ID
Payload Length

Version and flags are one-byte values and do not require endian conversion.

# 5. Decoder Development

The Decoder was implemented as a static decoding function:

Packet Decoder::decode(
    const std::vector<std::uint8_t>& data
)

The Decoder reads the fields from their defined offsets.

The header layout is:
Offset  Size    Field
------  ------  ----------------
0       4       Magic Number
4       1       Version
5       1       Flags
6       2       Opcode
8       4       Sequence ID
12      4       Payload Length
16      ...     Payload

# 6. Endian Conversion During Decoding

The Decoder uses:
common::Endian::networkToHost(...)
for multi-byte fields.

This is applied to:

* Magic number
* Opcode
* Sequence ID
* Payload length

This ensures that the values reconstructed by the Decoder are represented correctly on the host machine.

The resulting flow is:

Encoded bytes
      |
      v
Network byte order
      |
      v
Decoder
      |
      v
networkToHost()
      |
      v
Host representation

# 7. Debugging and Fixes

During Phase 3 development, several issues were encountered and resolved.

## 7.1 Encoder Member Function

Initially, the test attempted to call:
Encoder::encode(packet);

However, `encode()` is a non-static member function.

The correct usage is:
Encoder encoder;

auto data = encoder.encode(packet);

This was corrected in the decoder tests.

## 7.2 Passing Encoder Instead of Encoded Data

Another issue occurred when the Decoder was called using the Encoder object:

Decoder::decode(encoder);

The Decoder expects:

const std::vector<std::uint8_t>&

Therefore, the correct implementation is:

auto data = encoder.encode(packet);

auto decoded = Decoder::decode(data);

## 7.3 Magic Number Failure

A significant debugging issue occurred when the encoded magic number was inspected using GDB.

The encoded bytes were:

data[0] = 84  ('T')
data[1] = 75  ('K')
data[2] = 70  ('F')
data[3] = 80  ('P')

The bytes represented the network-order form of the magic number.

This exposed the importance of consistently applying endian conversion on both sides of the protocol boundary.

The final implementation uses:

Endian::hostToNetwork()

in the Encoder and:
Endian::networkToHost()

in the Decoder.

After this correction, both Encoder and Decoder tests passed.

# 8. Build Integration

The Makefile was updated/integrated so that the Encoder and Decoder test binaries link against the Endian object:

build/common/endian.o

For example, the Encoder test links:

packet.o
encoder.o
endian.o
test_encoder.o

The Decoder test links:

packet.o
encoder.o
decoder.o
endian.o
test_decoder.o

This is required because the protocol modules depend on the Endian implementation.
# 9. Unit Testing

Phase 3 includes dedicated tests for all three protocol modules.

## 9.1 Packet Tests

The Packet tests verify:

* Default packet validity.
* Default protocol version.
* Version modification.
* Flags modification.
* Opcode modification.
* Sequence ID modification.
* Empty payload behavior.
* Payload storage.
* Payload length calculation.

Result:
9 tests passed

## 9.2 Encoder Tests

The Encoder tests verify:

* Empty packet encoding.
* Payload encoding.
* Correct magic number.
* Correct version.
* Correct flags.
* Payload placement.

Result:
6 tests passed

## 9.3 Decoder Tests

The Decoder tests verify:

* Empty packet decoding.
* Payload decoding.
* Invalid magic number rejection.
* Invalid payload length rejection.

Result:

4 tests passed

# 10. Endian Tests

Because Phase 3 depends on the Endian module, the existing Endian tests were also executed.

The tests verify:

* Host endianness detection.
* 16-bit conversion.
* 32-bit conversion.
* 64-bit conversion.

Result:

4 tests passed

# 11. Complete Phase 3 Test Result

The complete Phase 3 test execution completed successfully.

Relevant results:

PacketTest       9/9   PASSED
EncoderTest      6/6   PASSED
DecoderTest      4/4   PASSED
EndianTest       4/4   PASSED

The other existing PacketForge tests also passed:

ErrorTest
LoggerTest
BufferTest
EndianTest
SocketTest
ConnectionTest
TransportTest
PacketTest
EncoderTest
DecoderTest

The final test run showed:

ALL TESTS PASSED

# 12. Phase 3 Completion Criteria

Phase 3 is considered complete because:

* [x] Packet module implemented.
* [x] Packet header format defined.
* [x] Variable-length payload supported.
* [x] Encoder implemented.
* [x] Decoder implemented.
* [x] Network byte order established.
* [x] Existing Endian module reused.
* [x] Encoder performs host-to-network conversion.
* [x] Decoder performs network-to-host conversion.
* [x] Invalid packet size is rejected.
* [x] Invalid magic number is rejected.
* [x] Invalid payload length is rejected.
* [x] Packet unit tests pass.
* [x] Encoder unit tests pass.
* [x] Decoder unit tests pass.
* [x] Endian unit tests pass.
* [x] Complete project test suite passes.

# 13. Phase 3 Final Architecture

The completed implementation can be represented as:

                         PacketForge
                             |
                  +----------+----------+
                  |                     |
               Protocol               Common
                  |                     |
       +----------+----------+       Endian
       |          |          |
     Packet    Encoder    Decoder
       |          |          |
       |          |          |
       |     hostToNetwork() |
       |          |          |
       |          |    networkToHost()
       |          |          |
       +----------+----------+
                  |
             Packet Format

The resulting architecture provides a clean boundary between:

* packet representation,
* serialization,
* deserialization,
* and byte-order handling.

This establishes the protocol foundation required for subsequent PacketForge development phases.
