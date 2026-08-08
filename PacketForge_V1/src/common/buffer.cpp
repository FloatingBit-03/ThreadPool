#include "common/buffer.hpp"


#include <algorithm>
#include <cstring>


namespace packetforge::common
{


Buffer::Buffer(
    std::size_t capacity)
{
    buffer_.reserve(capacity);
}



void Buffer::write(
    const std::uint8_t* data,
    std::size_t size)
{
    buffer_.insert(
        buffer_.end(),
        data,
        data + size);
}



bool Buffer::read(
    std::uint8_t* destination,
    std::size_t size)
{
    if (readableSize() < size)
    {
        return false;
    }


    std::memcpy(
        destination,
        buffer_.data() + readPosition_,
        size);


    readPosition_ += size;


    return true;
}



std::size_t Buffer::readableSize() const noexcept
{
    return buffer_.size() - readPosition_;
}



std::size_t Buffer::size() const noexcept
{
    return buffer_.size();
}



void Buffer::clear()
{
    buffer_.clear();

    readPosition_ = 0;
}



const std::vector<std::uint8_t>&
Buffer::data() const noexcept
{
    return buffer_;
}


}