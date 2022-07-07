#pragma once

#include <cstdint>
#include <limits>
#include <tuple>

static constexpr uintmax_t u1b = std::numeric_limits<uint_least8_t>::max();
static constexpr uintmax_t u2b = std::numeric_limits<uint_least16_t>::max();
static constexpr uintmax_t u4b = std::numeric_limits<uint_least32_t>::max();
static constexpr uintmax_t u8b = std::numeric_limits<uint_least64_t>::max();

static constexpr uintmax_t s1b = std::numeric_limits<int_least8_t>::max();
static constexpr uintmax_t s2b = std::numeric_limits<int_least16_t>::max();
static constexpr uintmax_t s4b = std::numeric_limits<int_least32_t>::max();
static constexpr uintmax_t s8b = std::numeric_limits<int_least64_t>::max();

template <uintmax_t N>
using MinUInt = std::tuple_element_t<(N > u1b) + (N > u2b) + (N > u4b) + (N > u8b), std::tuple<uint_least8_t, uint_least16_t, uint_least32_t, uint_least64_t, uintmax_t>>;

template <uintmax_t N>
using FastUInt = std::tuple_element_t<(N > u1b) + (N > u2b) + (N > u4b) + (N > u8b), std::tuple<uint_fast8_t, uint_fast16_t, uint_fast32_t, uint_fast64_t, uintmax_t>>;

template <intmax_t N>
using MinInt = std::tuple_element_t<(N > s1b) + (N > s2b) + (N > s4b) + (N > s8b), std::tuple<int_least8_t, int_least16_t, int_least32_t, int_least64_t, intmax_t>>;

template <intmax_t N>
using FastInt = std::tuple_element_t<(N > s1b) + (N > s2b) + (N > s4b) + (N > s8b), std::tuple<int_fast8_t, int_fast16_t, int_fast32_t, int_fast64_t, intmax_t>>;

template <intmax_t N, typename T>
using FastSum = std::tuple_element_t<std::is_signed<T>::value, std::tuple<FastInt<N * std::numeric_limits<T>::max()>, FastUInt<N * std::numeric_limits<T>::max()>>>;

// template <uintmax_t num>
// constexpr size_t number_of_digits = num >= -9 && num <= 9 ? 1 : 1 + number_of_digits<num / 10>;

// template <intmax_t num>
// constexpr size_t number_of_digits = num >= -9 && num <= 9 ? 1 : 1 + number_of_digits<num / 10>;
//