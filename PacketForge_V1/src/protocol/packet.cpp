#include "protocol/packet.hpp"

#include <limits>

namespace packetforge::protocol
{

Packet::Packet()
    :
    magic_(Packet::MagicNumber),
    version_(Packet::VERSION),
    flags_(0),
    opcode_(0),
    sequenceId_(0),
    payload_()
{
}

std::uint32_t
Packet::magicNumber() const noexcept
{
    return magic_;
}

void
Packet::setVersion(
    std::uint8_t version) noexcept
{
    version_ = version;
}

std::uint8_t
Packet::version() const noexcept
{
    return version_;
}

void
Packet::setFlags(
    std::uint8_t flags) noexcept
{
    flags_ = flags;
}

std::uint8_t
Packet::flags() const noexcept
{
    return flags_;
}

void
Packet::setOpcode(
    std::uint16_t opcode) noexcept
{
    opcode_ = opcode;
}

std::uint16_t
Packet::opcode() const noexcept
{
    return opcode_;
}

void
Packet::setSequenceId(
    std::uint32_t id) noexcept
{
    sequenceId_ = id;
}

std::uint32_t
Packet::sequenceId() const noexcept
{
    return sequenceId_;
}

std::uint32_t
Packet::payloadLength() const noexcept
{
    return static_cast<std::uint32_t>(payload_.size());
}

void
Packet::setPayload(
    const std::vector<std::uint8_t>& payload)
{
    payload_ = payload;
}

const std::vector<std::uint8_t>&
Packet::payload() const noexcept
{
    return payload_;
}

bool Packet::isValid() const noexcept
{
    if (magic_ != MagicNumber)
    {
        return false;
    }

    if (version_ != VERSION)
    {
        return false;
    }

    if (payload_.size() >
        std::numeric_limits<std::uint32_t>::max())
    {
        return false;
    }

    return true;
}
} // namespace packetforge::protocol