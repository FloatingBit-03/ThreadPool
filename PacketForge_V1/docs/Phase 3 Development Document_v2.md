PacketForge — Phase 3 Development Document

1. Overview
Phase 3 implements the PacketForge Protocol Layer.

The purpose of this phase is to establish a complete, testable packet representation and provide conversion between:

In-Memory Packet
       |
       v
    Encoder
       |
       v
 Wire-format bytes
       |
       v
    Decoder
       |
       v
In-Memory Packet

Phase 3 consists of:

Packet
Encoder
Decoder

The implementation also reuses the existing Endian module from the Common layer.

The development approach followed the PacketForge modular workflow:

Design interface
      |
      v
Write tests
      |
      v
Implement module
      |
      v
Run tests
      |
      v
Find and fix issues
      |
      v
Extend tests
      |
      v
Run complete suite
      |
      v
Freeze Phase 3

Phase 3 has now been implemented and verified successfully.

2. Phase 3 Scope
The Phase 3 implementation provides:

A fixed 16-byte packet header.
Packet metadata representation.
Variable-length binary payloads.
Network byte-order serialization.
Network byte-order deserialization.
Packet validation.
Invalid packet rejection.
Encoder validation of invalid packets.
Decoder validation of malformed wire data.
Unit tests for all protocol components.

The protocol header is:

+----------------------+--------+
| Field                | Size   |
+----------------------+--------+
| Magic Number         | 4      |
| Version              | 1      |
| Flags                | 1      |
| Opcode               | 2      |
| Sequence ID          | 4      |
| Payload Length       | 4      |
+----------------------+--------+
| Total Header         | 16     |
+----------------------+--------+
| Payload              | Variable
+----------------------+--------+

3. Source Files
The Phase 3 implementation consists of:

include/protocol/packet.hpp
src/protocol/packet.cpp

include/protocol/encoder.hpp
src/protocol/encoder.cpp

include/protocol/decoder.hpp
src/protocol/decoder.cpp

The protocol layer depends on:

include/common/endian.hpp
src/common/endian.cpp

Unit tests:
tests/unit/test_packet.cpp
tests/unit/test_encoder.cpp
tests/unit/test_decoder.cpp

4. Packet Module Development
4.1 Purpose

Packet is the in-memory representation of a PacketForge protocol packet.

The class stores:

Magic number
Protocol version
Flags
Opcode
Sequence ID
Payload

The payload length is derived from the actual payload:

std::uint32_t Packet::payloadLength() const noexcept
{
    return static_cast<std::uint32_t>(payload_.size());
}
This avoids maintaining a second mutable payload-length field that could become inconsistent with the payload itself.

5. Packet Defaults
A newly constructed Packet has:

Magic Number = Packet::MagicNumber
Version      = Packet::VERSION
Flags        = 0
Opcode       = 0
Sequence ID  = 0
Payload      = empty

Therefore a default packet is valid.
The default protocol version is explicitly represented by: Packet::VERSION

rather than duplicating the literal value throughout the implementation and tests.

6. Packet Validation
During Phase 3 development, packet validation was strengthened.

The Packet class provides: bool isValid() const noexcept;

Validation is used by the Encoder before serializing a packet.
An invalid packet must not be serialized.
One important case identified during development was an unsupported protocol version.

For example:
Packet packet;
packet.setVersion(2);

must produce an invalid Packet.
The corresponding encoder behavior is:encoder.encode(packet);

which rejects the invalid packet instead of producing a wire representation.
The unit tests verify this behavior.

7. Encoder Development
7.1 Purpose
The Encoder converts an in-memory Packet into a wire-format byte vector.
The interface is:

class Encoder
{
public:
    Encoder() = default;

    [[nodiscard]]
    std::vector<std::uint8_t>
    encode(const Packet& packet) const;
};

The Encoder is intentionally an object with a member function rather than a static function.
Correct usage:Encoder encoder;

auto data = encoder.encode(packet);

During development, this distinction was important because an earlier test attempted to treat encode() as static.
The test was corrected to use an Encoder instance.

8. Encoder Serialization Order

The Encoder writes fields in the exact protocol order:

Magic Number
Version
Flags
Opcode
Sequence ID
Payload Length
Payload

The resulting byte layout is:

Offset   Size   Field
--------------------------------
0        4      Magic Number
4        1      Version
5        1      Flags
6        2      Opcode
8        4      Sequence ID
12       4      Payload Length
16       N      Payload

The fixed header therefore occupies exactly: 16 bytes

9. Encoder Validation
Before serialization, the Encoder verifies that the Packet is valid.
An invalid Packet must not be encoded.
The test suite explicitly verifies unsupported protocol versions: packet.setVersion(2);

EXPECT_FALSE(packet.isValid());

EXPECT_THROW(
    encoder.encode(packet),
    std::invalid_argument
);

This establishes an important protocol boundary:
Packet
  |
  | invalid
  v
Encoder
  |
  X
Rejected

rather than:
Invalid Packet
      |
      v
Incorrect wire data

10. Endian Integration
One of the important implementation issues identified during Phase 3 was byte ordering.
PacketForge uses network byte order for multi-byte protocol fields.
The existing Common Endian module is reused rather than duplicating byte-order conversion logic.

The Encoder uses:common::Endian::hostToNetwork(...)

for:
Magic number
Opcode
Sequence ID
Payload length

The following fields do not require conversion because they are one byte:
Version
Flags

11. Magic Number Endianness Issue

During development, the encoded magic number was inspected at the byte level.
The resulting bytes appeared in network order:84 75 70 80
which correspond to:'T' 'K' 'F' 'P'

This initially made the encoded representation appear reversed when viewed directly as bytes on a little-endian host.
The issue was resolved by treating the wire format and host representation as separate representations.

The final rule is:
Host representation
       |
       | hostToNetwork()
       v
Network representation

and on decoding:
Network representation
       |
       | networkToHost()
       v
Host representation

This establishes a platform-independent wire format.

12. Decoder Development
12.1 Purpose
The Decoder converts encoded protocol bytes back into a Packet.

Its interface is:
static Packet decode(
    const std::vector<std::uint8_t>& data
);

Example: Encoder encoder;

auto data = encoder.encode(packet);

auto decoded = Decoder::decode(data);

During development, the distinction between the Encoder object and the encoded byte vector was clarified.

The Decoder accepts: std::vector<std::uint8_t>
not an Encoder object.

13. Decoder Processing

The Decoder performs the following operations:
Verify minimum packet size.
Read the magic number.
Convert the magic number from network to host order.
Validate the magic number.
Read the version.
Read the flags.
Read the opcode.
Convert the opcode to host order.
Read the sequence ID.
Convert the sequence ID to host order.
Read the payload length.
Convert the payload length to host order.
Validate the complete packet size.
Extract the payload.
Return the decoded Packet.

The complete flow is:
Byte Vector
     |
     v
Header Size Validation
     |
     v
Magic Number
     |
     v
networkToHost()
     |
     v
Magic Validation
     |
     v
Read Remaining Header
     |
     v
networkToHost()
     |
     v
Payload Length Validation
     |
     v
Payload Extraction
     |
     v
Packet

14. Decoder Validation
The Decoder validates external byte data before accepting it as a Packet.

14.1 Insufficient Header
If: data.size() < Packet::HEADER_SIZE
decoding fails.

The decoder reports:Invalid packet size

14.2 Invalid Magic Number
The Decoder reads the first four bytes and converts them from network byte order.
If the result does not equal: Packet::MagicNumber
decoding fails.

The decoder reports:Invalid magic number

14.3 Payload Length Mismatch
The decoder reads the declared payload length from the header.

The expected packet size is:Packet::HEADER_SIZE + payloadLength

The actual input must satisfy:data.size() == Packet::HEADER_SIZE + payloadLength

Otherwise decoding fails with: Payload size mismatch
This prevents malformed or truncated packets from being accepted.

15. Decoder Error Handling
The current Decoder interface reports malformed wire data through exceptions.

Examples include:
std::runtime_error("Invalid packet size");
std::runtime_error("Invalid magic number");
std::runtime_error("Payload size mismatch");

This behavior is covered by the unit tests.
The higher-level Client and Server layers can later decide how these protocol errors should be mapped into the PacketForge common::Error model.
That decision is intentionally left outside Phase 3.

16. Unit Testing Strategy
Phase 3 follows the project's test-driven modular workflow.
Each protocol module has an independent test suite.

Packet
  |
  +--> test_packet.cpp

Encoder
  |
  +--> test_encoder.cpp

Decoder
  |
  +--> test_decoder.cpp

The tests verify both normal behavior and invalid input handling.

17. Packet Tests
The Packet test suite verifies:

Default packet validity.
Default version.
Version modification.
Flags modification.
Opcode modification.
Sequence ID modification.
Empty payload.
Payload storage.
Payload length calculation.

Final result:
9 tests
9 passed
0 failed

18. Encoder Tests
The Encoder test suite was expanded during Phase 3 development.

It verifies:
Empty packet encoding.
Payload encoding.
Correct magic number.
Correct protocol version.
Rejection of invalid version.
Correct flags.
Correct payload placement.
Correct opcode serialization.
Correct sequence ID serialization.
Correct payload length serialization.
Complete packet encoding.

Final result:

11 tests
11 passed
0 failed

Important header-field tests inspect the actual wire representation and convert multi-byte values back using: Endian::networkToHost(...)
This verifies not only the Packet API but also the actual protocol byte layout.

19. Decoder Tests
The Decoder test suite verifies:

Empty packet decoding.
Payload decoding.
Invalid magic rejection.
Invalid payload length rejection.

Final result:
4 tests
4 passed
0 failed

20. Endian Tests
The existing Endian tests were also executed because the Protocol layer depends directly on them.

The tests verify:
Host-endianness detection.
16-bit conversion.
32-bit conversion.
64-bit conversion.

Final result:
4 tests
4 passed
0 failed

21. Complete Test Suite
The complete PacketForge unit test run was executed after the Phase 3 implementation was finalized.

The test suites reported:

ErrorTest: 4/4   PASSED
LoggerTest: 5/5   PASSED
BufferTest: 3/3   PASSED
EndianTest: 4/4   PASSED
SocketTest: 5/5   PASSED
ConnectionTest: 9/9   PASSED
TransportTest: 5/5   PASSED
PacketTest: 9/9   PASSED
EncoderTest: 11/11 PASSED
DecoderTest: 4/4   PASSED

Final status: ALL TESTS PASSED

This confirms that the Phase 3 implementation did not introduce regressions into the previously implemented Common and Network layers.

22. Development Issues Resolved
Several implementation issues were discovered and resolved during Phase 3.

22.1 Encoder Instance Usage
The Encoder API is: encode(const Packet& packet) const
as a non-static member function.

Therefore: Encoder::encode(packet);
is incorrect.

The correct usage is:Encoder encoder;
auto data = encoder.encode(packet);

22.2 Decoder Input
The Decoder expects encoded bytes:const std::vector<std::uint8_t>&

Therefore this is incorrect:Decoder::decode(encoder);

The correct flow is:
auto data = encoder.encode(packet);
auto decoded = Decoder::decode(data);

22.3 Protocol Endianness
Direct inspection of encoded multi-byte fields exposed the difference between host representation and wire representation.
The final implementation consistently uses:

Encoder:hostToNetwork()
Decoder:networkToHost()

for all multi-byte protocol fields.

22.4 Invalid Version Handling
An unsupported version was identified as a protocol validity issue.
The tests were expanded to explicitly verify:
packet.setVersion(2);
EXPECT_FALSE(packet.isValid());

and:
EXPECT_THROW(
    encoder.encode(packet),
    std::invalid_argument
);

This ensures that invalid protocol packets cannot silently enter the wire format.

22.5 Header Field Coverage
The Encoder tests were extended beyond the initial magic/version/flags coverage.

The final tests explicitly verify:
Magic Number
Version
Flags
Opcode
Sequence ID
Payload Length
Payload

This provides direct coverage of the complete packet header.

23. Build Integration
The Protocol modules depend on the Common Endian implementation.

Therefore the relevant test targets must link:
packet.o
encoder.o
decoder.o
endian.o

as appropriate.

The dependency relationship is:
Encoder
   |
   +----> Endian

Decoder
   |
   +----> Endian

This keeps byte-order implementation centralized in the Common layer.

24. Phase 3 Completion Criteria
Phase 3 satisfies the planned completion criteria:

[x] Packet interface implemented
[x] Packet implementation completed
[x] Fixed 16-byte header defined
[x] Magic number defined
[x] Protocol version defined
[x] Flags implemented
[x] Opcode implemented
[x] Sequence ID implemented
[x] Variable-length payload implemented
[x] Payload length derived from payload
[x] Packet validation implemented

[x] Encoder interface implemented
[x] Encoder implementation completed
[x] Packet validation before encoding
[x] Network byte-order serialization
[x] Complete header serialization
[x] Payload serialization

[x] Decoder interface implemented
[x] Decoder implementation completed
[x] Network byte-order deserialization
[x] Header-size validation
[x] Magic-number validation
[x] Payload-length validation
[x] Payload extraction

[x] Endian module integrated
[x] Packet tests passing
[x] Encoder tests passing
[x] Decoder tests passing
[x] Complete existing test suite passing

25. Final Phase 3 Architecture

The completed Phase 3 architecture is:

                         PacketForge
                              |
                 +------------+------------+
                 |                         |
              Protocol                   Common
                 |                         |
       +---------+---------+             Endian
       |         |         |                |
    Packet    Encoder   Decoder             |
       |         |         |                |
       |         +----+----+----------------+
       |              |
       |       hostToNetwork()
       |       networkToHost()
       |              |
       +--------------+
              |
        Packet Format

The responsibilities are clearly separated:

Packet
    |
    +-- In-memory representation

Encoder
    |
    +-- Packet -> wire bytes

Decoder
    |
    +-- wire bytes -> Packet

Endian
    |
    +-- Host/network byte-order conversion

26. Phase 3 Freeze State
Phase 3 is now considered implementation-complete and frozen.

The frozen protocol boundary is:

Packet
   |
   v
Encoder
   |
   v
std::vector<std::uint8_t>
   |
   v
Decoder
   |
   v
Packet

The protocol wire format is fixed at:
Header = 16 bytes

0   - 3    Magic Number
4          Version
5          Flags
6   - 7    Opcode
8   - 11   Sequence ID
12  - 15   Payload Length
16+        Payload

The current Phase 3 implementation has passed the complete available test suite.

No Phase 4 client/server behavior is included in this phase.

27. Transition to Phase 4

With Phase 3 frozen, development can proceed to the Client layer.
Phase 4 will build on the existing:
Common
   |
Network
   |
Protocol
   |
   +---- Packet
   +---- Encoder
   +---- Decoder

The Client layer will therefore consume the already-established protocol API rather than modifying the protocol implementation.

The intended Phase 4 relationship is:

                    Client
                      |
          +-----------+-----------+
          |                       |
       Network                Protocol
          |                       |
       Socket              +------+------+ 
                           |             |
                        Encoder       Decoder
                           |             |
                           +------+------+
                                  |
                                Packet

Phase 4 design should therefore begin with the Client public API, followed by checking that API against the frozen Network and Protocol interfaces before implementation of client.cpp.
This preserves the bottom-up development strategy established by PacketForge.