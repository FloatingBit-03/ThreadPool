#pragma once

#include <cstddef>
#include <cstdint>
#include <vector>


#include "common/error.hpp"
#include "network/connection.hpp"



namespace packetforge::network
{


class Transport
{

public:


    Transport();

    explicit Transport(
        Connection connection
    );


    ~Transport();



    // Non-copyable

    Transport(const Transport&) = delete;

    Transport& operator=(
        const Transport&
    ) = delete;



    // Movable

    Transport(Transport&& other) noexcept;

    Transport& operator=(
        Transport&& other
    ) noexcept;



    // Data Transfer


    common::Error send(
        const std::vector<std::uint8_t>& data
    );



    common::Error receive(
        std::vector<std::uint8_t>& data,
        std::size_t size
    );



    // State


    bool isConnected() const noexcept;



    Connection& connection() noexcept;



private:

    Connection connection_;

};


}