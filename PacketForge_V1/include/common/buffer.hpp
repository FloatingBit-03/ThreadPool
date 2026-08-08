#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>


namespace packetforge::common
{


class Buffer
{
public:

    Buffer() = default;


    explicit Buffer(
        std::size_t capacity);


    // Write raw bytes into buffer
    void write(
        const std::uint8_t* data,
        std::size_t size);


    // Read bytes from buffer
    bool read(
        std::uint8_t* destination,
        std::size_t size);


    // Current readable bytes
    std::size_t readableSize() const noexcept;


    // Total stored bytes
    std::size_t size() const noexcept;


    // Remove already read bytes
    void clear();


    // Access raw data
    const std::vector<std::uint8_t>& data() const noexcept;


private:

    std::vector<std::uint8_t> buffer_;

    std::size_t readPosition_{0};

};


}