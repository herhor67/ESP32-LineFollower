#pragma once

#include <cstdint>



template<uint64_t N>
struct constant
{
    enum { value = N };
};

template<typename T>
struct return_
{
    typedef T type;
};


template<uint64_t N>
struct bitcount : constant<1 + bitcount<(N >> 1)>::value> {};

template<>
struct bitcount<0> : constant<1> {};

template<>
struct bitcount<1> : constant<1> {};

template<uint64_t N>
struct bytecount : constant<((bitcount<N>::value + 7) >> 3)> {};


template<uint64_t N>
struct bytetype : return_<uint64_t> {};

template<>
struct bytetype<4> : return_<uint32_t> {};

template<>
struct bytetype<3> : return_<uint32_t> {};

template<>
struct bytetype<2> : return_<uint16_t> {};

template<>
struct bytetype<1> : return_<uint8_t> {};


template<uint64_t N>
struct MinUInt : bytetype<bytecount<N>::value> {};