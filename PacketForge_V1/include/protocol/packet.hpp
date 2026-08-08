#pragma once

#include <cstdint>
#include <vector>

namespace packetforge::protocol
{

class Packet
{
public:

    static constexpr std::size_t HEADER_SIZE = 16;

    static constexpr std::uint32_t MagicNumber = 0x50464B54;
    // "PFKT"

    Packet();

    // Magic Number

    std::uint32_t magicNumber() const noexcept;

    // Version

    void setVersion(std::uint8_t version) noexcept;

    std::uint8_t version() const noexcept;

    // Flags

    void setFlags(std::uint8_t flags) noexcept;

    std::uint8_t flags() const noexcept;

    // Opcode

    void setOpcode(std::uint16_t opcode) noexcept;

    std::uint16_t opcode() const noexcept;

    // Sequence ID

    void setSequenceId(std::uint32_t id) noexcept;

    std::uint32_t sequenceId() const noexcept;

    // Payload

    void setPayload(
        const std::vector<std::uint8_t>& payload);

    const std::vector<std::uint8_t>&
    payload() const noexcept;

    std::uint32_t payloadLength() const noexcept;

    // Validation

    bool isValid() const noexcept;

private:

    std::uint32_t magic_;

    std::uint8_t version_;

    std::uint8_t flags_;

    std::uint16_t opcode_;

    std::uint32_t sequenceId_;

    std::vector<std::uint8_t> payload_;

};

} // namespace packetforge::protocol