#pragma once

#include <bitset>
#include <algorithm>
#include <optional>

template <size_t N>
std::bitset<N> rotl(const std::bitset<N> &bits, size_t c)
{
	c = (c >= N) ? c % N : c;
	return bits << c | bits >> (N - c);
}
template <size_t N>
std::bitset<N> rotr(const std::bitset<N> &bits, size_t c)
{
	c = (c >= N) ? c % N : c;
	return bits >> c | bits << (N - c);
}

template <size_t N>
std::bitset<N> rotl1(const std::bitset<N> &bits)
{
	if (bits.test(N - 1))
		return (bits << 1).set(1);
	else
		return (bits << 1);
}

template <size_t N>
std::bitset<N> rotr1(const std::bitset<N> &bits)
{
	if (bits.test(0))
		return (bits >> 1).set(N - 1);
	else
		return (bits >> 1);
}

/*/
template <size_t toN, size_t fromN>
constexpr std::bitset<toN> bitset_cast(std::bitset<fromN> start)
{
	using word = typename std::bitset<fromN>::_WordT;
	constexpr size_t word_w = CHAR_BIT * sizeof(word);
	constexpr size_t word_c = fromN == 0 ? 0 : (fromN - 1) / word_w;

	std::bitset<toN> result;

	for (size_t i = 0; i <= word_c; ++i)
	{
		result <<= word_w;
		result |= start._Getword(word_c - i);
	}
	return result;
}
//*/

/*/
template <size_t toN = 556, size_t fromN = 160>
constexpr std::bitset<toN> convert(std::bitset<fromN> start)
{
	constexpr size_t undrlng_t_w = std::min(std::bitset<fromN>::_Bitsperword, std::bitset<toN>::_Bitsperword);
	using word = std::bitset<fromN>::_Ty;

	constexpr size_t word_w = sizeof(word) * CHAR_BIT;
	constexpr word mask = ~0;
	constexpr size_t minBits = (fromN < toN) ? fromN : toN;
	constexpr size_t minWords = (minBits - 1) / word_w + 1;

	std::bitset<toN> result;

	for (size_t i = 0; i < minWords; ++i)
	{
		result <<= word_w;
		piece =
			if constexpr (word_w == 64)
				result |= start.to_ullong();
			else if constexpr (word_w == 32)
				result |= start.to_ulong();
		start >>= undrlng_t_w;
	}
	return result;
}
//*/

template <size_t N>
constexpr std::optional<size_t> find_last_index_of_bit_set(std::bitset<N> set)
{
	for (size_t i = set.size() - 1; i != -1; --i)
		if (set.test(i))
			return std::optional(i);

	return std::nullopt;
}