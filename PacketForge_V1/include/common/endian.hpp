#pragma once

#include <cstdint>

namespace packetforge::common
{

enum class Endianness
{
    Little,
    Big
};


class Endian
{
public:

    static Endianness hostEndianness() noexcept;

    static std::uint16_t hostToNetwork(
        std::uint16_t value) noexcept;

    static std::uint32_t hostToNetwork(
        std::uint32_t value) noexcept;

    static std::uint64_t hostToNetwork(
        std::uint64_t value) noexcept;

    static std::uint16_t networkToHost(
        std::uint16_t value) noexcept;

    static std::uint32_t networkToHost(
        std::uint32_t value) noexcept;

    static std::uint64_t networkToHost(
        std::uint64_t value) noexcept;

private:

    static std::uint16_t swap16(
        std::uint16_t value) noexcept;

    static std::uint32_t swap32(
        std::uint32_t value) noexcept;

    static std::uint64_t swap64(
        std::uint64_t value) noexcept;
};

}