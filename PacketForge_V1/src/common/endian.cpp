#include "common/endian.hpp"

#include <bit>

namespace packetforge::common
{

Endianness
Endian::hostEndianness() noexcept
{
    return (std::endian::native == std::endian::little)
               ? Endianness::Little
               : Endianness::Big;
}

std::uint16_t
Endian::swap16(
    std::uint16_t value) noexcept
{
    return (value >> 8) |
           (value << 8);
}

std::uint32_t
Endian::swap32(
    std::uint32_t value) noexcept
{
    return ((value & 0x000000FFu) << 24) |
           ((value & 0x0000FF00u) << 8)  |
           ((value & 0x00FF0000u) >> 8)  |
           ((value & 0xFF000000u) >> 24);
}

std::uint64_t
Endian::swap64(
    std::uint64_t value) noexcept
{
    return ((value & 0x00000000000000FFULL) << 56) |
           ((value & 0x000000000000FF00ULL) << 40) |
           ((value & 0x0000000000FF0000ULL) << 24) |
           ((value & 0x00000000FF000000ULL) << 8)  |
           ((value & 0x000000FF00000000ULL) >> 8)  |
           ((value & 0x0000FF0000000000ULL) >> 24) |
           ((value & 0x00FF000000000000ULL) >> 40) |
           ((value & 0xFF00000000000000ULL) >> 56);
}

std::uint16_t
Endian::hostToNetwork(
    std::uint16_t value) noexcept
{
    if (hostEndianness() == Endianness::Little)
    {
        return swap16(value);
    }

    return value;
}

std::uint32_t
Endian::hostToNetwork(
    std::uint32_t value) noexcept
{
    if (hostEndianness() == Endianness::Little)
    {
        return swap32(value);
    }

    return value;
}

std::uint64_t
Endian::hostToNetwork(
    std::uint64_t value) noexcept
{
    if (hostEndianness() == Endianness::Little)
    {
        return swap64(value);
    }

    return value;
}

std::uint16_t
Endian::networkToHost(
    std::uint16_t value) noexcept
{
    return hostToNetwork(value);
}

std::uint32_t
Endian::networkToHost(
    std::uint32_t value) noexcept
{
    return hostToNetwork(value);
}

std::uint64_t
Endian::networkToHost(
    std::uint64_t value) noexcept
{
    return hostToNetwork(value);
}

} // namespace packetforge::common

