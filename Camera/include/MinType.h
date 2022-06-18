#pragma once

#include <cstdint>
#include <limits>
#include <tuple>

static constexpr uintmax_t m1b = std::numeric_limits<uint_least8_t>::max();
static constexpr uintmax_t m2b = std::numeric_limits<uint_least16_t>::max();
static constexpr uintmax_t m4b = std::numeric_limits<uint_least32_t>::max();
static constexpr uintmax_t m8b = std::numeric_limits<uint_least64_t>::max();

template <uintmax_t N>
using MinUInt = std::tuple_element_t<(N > m1b) + (N > m2b) + (N > m4b) + (N > m8b), std::tuple<uint_least8_t, uint_least16_t, uint_least32_t, uint_least64_t, uintmax_t>>;
