#pragma once

#include <iostream>
#include <array>
#include <bitset>
#include <iterator>
#include <stdexcept>
#include <type_traits>

#include "MinType.h"

template <size_t H, size_t W>
class BitMatrix
{
	static_assert(H >= 1, "Height must be greater than 0!");
	static_assert(W >= 1, "Width  must be greater than 0!");

	using row = std::bitset<W>;
	using this_t = BitMatrix<H, W>;
	using crd_t = MinUInt<std::max(H, W)>;
	using idx_t = MinUInt<H * W>;

	std::array<row, H> content;

public:
	BitMatrix() {}
	~BitMatrix() {}

	constexpr row &operator[](size_t pos) const
	{
		return content[pos];
	}

	constexpr bool operator()(size_t y, size_t x) const
	{
		return content[y][W - x - 1];
	}

	typename row::reference operator()(size_t y, size_t x)
	{
		//		std::bitset<W>::reference temp = content[y][W - x - 1];
		//		return temp;
		return content[y][W - x - 1];
	}

	constexpr bool operator()(size_t pos) const
	{
		return content[pos / W][W - pos % W - 1];
	}

	typename row::reference operator()(size_t pos)
	{
		return content[pos / W][W - pos % W - 1];
	}

	// Contradiction           | 0    | 0
	this_t &Opq()
	{
		for (size_t y = 0; y < H; ++y)
			content[y].reset();
		return *this;
	}

	template <size_t Hb, size_t Wb> // Logical conjunction     | A∧B | A&B
	this_t &Kpq(const BitMatrix<Hb, Wb> &B)
	{
		static_assert(H == Hb, "Matrix heights must match!");
		static_assert(W == Wb, "Matrix widths  must match!");

		for (size_t y = 0; y < H; ++y)
			content[y] &= B.content[y];
		return *this;
	}

	template <size_t Hb, size_t Wb> // Material nonimplication | A↛B  | A&~B
	this_t &Lpq(const BitMatrix<Hb, Wb> &B)
	{
		static_assert(H == Hb, "Matrix heights must match!");
		static_assert(W == Wb, "Matrix widths  must match!");

		for (size_t y = 0; y < H; ++y)
			content[y] &= ~B.content[y];
		return *this;
	}

	// Projection function     | A    | A
	this_t &Ipq()
	{
		return *this;
	}

	template <size_t Hb, size_t Wb> // Converse nonimplication | A↚B  | ~A&B
	this_t &Mpq(const BitMatrix<Hb, Wb> &B)
	{
		static_assert(H == Hb, "Matrix heights must match!");
		static_assert(W == Wb, "Matrix widths  must match!");

		for (size_t y = 0; y < H; ++y)
		{
			content[y].flip();
			content[y] &= ~B.content[y];
		}
		return *this;
	}

	template <size_t Hb, size_t Wb> // Projection function     | B    | B
	this_t &Hpq(const BitMatrix<Hb, Wb> &B)
	{
		static_assert(H == Hb, "Matrix heights must match!");
		static_assert(W == Wb, "Matrix widths  must match!");

		for (size_t y = 0; y < H; ++y)
			content[y] = B.content[y];
		return *this;
	}

	template <size_t Hb, size_t Wb> // Exclusive disjunction   | A⊕B  | A^B
	this_t &Jpq(const BitMatrix<Hb, Wb> &B)
	{
		static_assert(H == Hb, "Matrix heights must match!");
		static_assert(W == Wb, "Matrix widths  must match!");

		for (size_t y = 0; y < H; ++y)
			content[y] ^= B.content[y];
		return *this;
	}

	template <size_t Hb, size_t Wb> // Logical disjunction     | A∨B | A|B
	this_t &Apq(const BitMatrix<Hb, Wb> &B)
	{
		static_assert(H == Hb, "Matrix heights must match!");
		static_assert(W == Wb, "Matrix widths  must match!");

		for (size_t y = 0; y < H; ++y)
			content[y] |= B.content[y];
		return *this;
	}

	template <size_t Hb, size_t Wb> // Logical NOR             | A↓B  | ~(A|B)
	this_t &Xpq(const BitMatrix<Hb, Wb> &B)
	{
		static_assert(H == Hb, "Matrix heights must match!");
		static_assert(W == Wb, "Matrix widths  must match!");

		for (size_t y = 0; y < H; ++y)
		{
			content[y] &= B.content[y];
			content[y].flip();
		}
		return *this;
	}

	template <size_t Hb, size_t Wb> // Logical biconditional   | A↔B  | ~(A^B)
	this_t &Epq(const BitMatrix<Hb, Wb> &B)
	{
		static_assert(H == Hb, "Matrix heights must match!");
		static_assert(W == Wb, "Matrix widths  must match!");

		for (size_t y = 0; y < H; ++y)
		{
			content[y] ^= B.content[y];
			content[y].flip();
		}
		return *this;
	}

	template <size_t Hb, size_t Wb> // Negation                | ¬B   | ~B
	this_t &Gpq(const BitMatrix<Hb, Wb> &B)
	{
		static_assert(H == Hb, "Matrix heights must match!");
		static_assert(W == Wb, "Matrix widths  must match!");

		for (size_t y = 0; y < H; ++y)
			content[y] = ~B.content[y];
		return *this;
	}

	template <size_t Hb, size_t Wb> // Converse implication    | A←B  | A|~B
	this_t &Bpq(const BitMatrix<Hb, Wb> &B)
	{
		static_assert(H == Hb, "Matrix heights must match!");
		static_assert(W == Wb, "Matrix widths  must match!");

		for (size_t y = 0; y < H; ++y)
			content[y] |= ~B.content[y];
		return *this;
	}

	// Negation                | ¬A   | ~A
	this_t &Fpq()
	{
		for (size_t y = 0; y < H; ++y)
			content[y].flip();
		return *this;
	}

	template <size_t Hb, size_t Wb> // Material implication    | A→B  | ~A|B
	this_t &Cpq(const BitMatrix<Hb, Wb> &B)
	{
		static_assert(H == Hb, "Matrix heights must match!");
		static_assert(W == Wb, "Matrix widths  must match!");

		for (size_t y = 0; y < H; ++y)
		{
			content[y].flip();
			content[y] |= B.content[y];
		}
		return *this;
	}

	template <size_t Hb, size_t Wb> // Logical NAND            | A↑B  | ~(A&B)
	this_t &Dpq(const BitMatrix<Hb, Wb> &B)
	{
		static_assert(H == Hb, "Matrix heights must match!");
		static_assert(W == Wb, "Matrix widths  must match!");

		for (size_t y = 0; y < H; ++y)
		{
			content[y] &= B.content[y];
			content[y].flip();
		}
		return *this;
	}

	// Tautology               | 1    | 1
	this_t &Vpq()
	{
		for (size_t y = 0; y < H; ++y)
			content[y].set();
		return *this;
	}

	this_t &flip()
	{
		return Fpq();
	}

	this_t &operator-()
	{
		return Opq();
	}
	this_t &operator+()
	{
		return Vpq();
	}
	this_t &operator~()
	{
		return Fpq();
	}
	this_t operator!() const
	{
		this_t temp(*this);
		return temp.Fpq();
	}

	template <size_t Hb, size_t Wb>
	this_t &operator|=(const BitMatrix<Hb, Wb> &B)
	{
		return Apq(B);
	}

	template <size_t Hb, size_t Wb>
	this_t &operator&=(const BitMatrix<Hb, Wb> &B)
	{
		return Kpq(B);
	}

	template <size_t Hb, size_t Wb>
	this_t &operator^=(const BitMatrix<Hb, Wb> &B)
	{
		return Jpq(B);
	}

	template <size_t Hb, size_t Wb>
	this_t &operator-=(const BitMatrix<Hb, Wb> &B)
	{
		return Lpq(B);
	}

	template <size_t Hb, size_t Wb>
	this_t &operator/=(const BitMatrix<Hb, Wb> &B)
	{
		return Bpq(B);
	}

	template <size_t Ha, size_t Wa, size_t Hb, size_t Wb>
	friend BitMatrix<Ha, Wa> operator|(BitMatrix<Ha, Wa> A, const BitMatrix<Hb, Wb> &B)
	{
		return A.operator|=(B);
	}

	template <size_t Ha, size_t Wa, size_t Hb, size_t Wb>
	friend BitMatrix<Ha, Wa> operator&(BitMatrix<Ha, Wa> A, const BitMatrix<Hb, Wb> &B)
	{
		return A.operator&=(B);
	}

	template <size_t Ha, size_t Wa, size_t Hb, size_t Wb>
	friend BitMatrix<Ha, Wa> operator^(BitMatrix<Ha, Wa> A, const BitMatrix<Hb, Wb> &B)
	{
		return A.operator^=(B);
	}

	template <size_t Ha, size_t Wa, size_t Hb, size_t Wb>
	friend BitMatrix<Ha, Wa> operator-(BitMatrix<Ha, Wa> A, const BitMatrix<Hb, Wb> &B)
	{
		return A.operator-=(B);
	}

	template <size_t Ha, size_t Wa, size_t Hb, size_t Wb>
	friend BitMatrix<Ha, Wa> operator/(BitMatrix<Ha, Wa> A, const BitMatrix<Hb, Wb> &B)
	{
		return A.operator/=(B);
	}

	template <size_t Ha, size_t Wa, size_t Hb, size_t Wb>
	friend BitMatrix<Ha, Wa> operator||(BitMatrix<Ha, Wa> A, const BitMatrix<Hb, Wb> &B)
	{
		return A.Xpq(B);
	}

	template <size_t Ha, size_t Wa, size_t Hb, size_t Wb>
	friend BitMatrix<Ha, Wa> operator&&(BitMatrix<Ha, Wa> A, const BitMatrix<Hb, Wb> &B)
	{
		return A.Dpq(B);
	}

	template <size_t Ha, size_t Wa, size_t Hb, size_t Wb>
	friend BitMatrix<Ha, Wa> operator==(BitMatrix<Ha, Wa> A, const BitMatrix<Hb, Wb> &B)
	{
		return A.Epq(B);
	}

	template <size_t Ha, size_t Wa, size_t Hb, size_t Wb>
	friend BitMatrix<Ha, Wa> operator!=(BitMatrix<Ha, Wa> A, const BitMatrix<Hb, Wb> &B)
	{
		return A.Jpq(B);
	}

	bool any() const
	{
		for (size_t y = 0; y < H; ++y)
			if (content[y].any())
				return true;
		return false;
	}

	bool all() const
	{
		for (size_t y = 0; y < H; ++y)
			if (!content[y].all())
				return false;
		return true;
	}

	bool none() const
	{
		for (size_t y = 0; y < H; ++y)
			if (!content[y].none())
				return false;
		return true;
	}

	template <size_t Ha, size_t Wa>
	friend std::ostream &operator<<(std::ostream &os, const BitMatrix<Ha, Wa> &A)
	{
		for (size_t y = 0; y < H; ++y)
			os << A.content[y].to_string('.', '#') << std::endl;
		os << "Any: " << A.any() << ", All: " << A.all() << ", None: " << A.none() << std::endl
		   << std::endl;

		return os;
	}

	constexpr this_t &dilate(char conn = '4', size_t num = 1)
	{
		if (conn == '8' || conn == 'o' || conn == 'O')
		{
			for (; num > 0; --num)
			{
				for (size_t y = 0; y < H; ++y)
				{
					content[y] |= content[y] << 1 | content[y] >> 1;
				}

				if constexpr (H == 1)
				{
				}
				else
				{
					row prev = content[0];
					content[0] |= content[1];

					for (size_t y = 1; y < H - 1; ++y)
					{
						row temp = content[y];
						content[y] |= content[y + 1] | prev;
						prev = temp;
					}

					content[H - 1] |= prev;
				}
			}
		}
		else if (conn == '4' || conn == '+')
		{
			for (; num > 0; --num)
			{
				if constexpr (H == 1)
				{
					content[0] |= content[0] << 1 | content[0] >> 1;
				}
				else
				{
					row prev = content[0];
					content[0] |= content[0] << 1 | content[0] >> 1 | content[1];

					for (size_t y = 1; y < H - 1; ++y)
					{
						row temp = content[y];
						content[y] |= content[y] << 1 | content[y] >> 1 | content[y + 1] | prev;
						prev = temp;
					}

					content[H - 1] |= content[H - 1] << 1 | content[H - 1] >> 1 | prev;
				}
			}
		}
		// else
		// throw std::invalid_argument("Invalid connectivity/shape! Use 4/+ or 8/o");

		return *this;
	}
	constexpr this_t &erode(char conn = '4', size_t num = 1)
	{
		if (conn == '8' || conn == 'o' || conn == 'O')
		{
			for (; num > 0; --num)
			{
				for (size_t y = 0; y < H; ++y)
				{
					content[y] &= content[y] << 1 & content[y] >> 1;
				}

				if constexpr (H == 1)
				{
					content[0].reset();
				}
				else
				{
					row prev = content[0];
					content[0].reset();

					for (size_t y = 1; y < H - 1; ++y)
					{
						row temp = content[y];
						content[y] &= content[y + 1] & prev;
						prev = temp;
					}

					content[H - 1].reset();
				}
			}
		}
		else if (conn == '4' || conn == '+')
		{
			for (; num > 0; --num)
			{
				if constexpr (H == 1)
				{
					content[0].reset();
				}
				else
				{
					row prev = content[0];
					content[0].reset();

					for (size_t y = 1; y < H - 1; ++y)
					{
						row temp = content[y];
						content[y] &= content[y] << 1 & content[y] >> 1 & content[y + 1] & prev;
						prev = temp;
					}

					content[H - 1].reset();
				}
			}
		}
		// else
		// throw std::invalid_argument("Invalid connectivity/shape! Use 4/+ or 8/o");

		return *this;
	}

	constexpr this_t dilation(char conn = '4', size_t num = 1) const
	{
		this_t temp(*this);
		temp.dilate(conn, num);
		return temp;
	}
	constexpr this_t erosion(char conn = '4', size_t num = 1) const
	{
		this_t temp(*this);
		temp.erode(conn, num);
		return temp;
	}
};

// template <typename T, size_t H, size_t W, std::enable_if_t<std::conjunction_v<std::is_arithmetic<T>, std::negation<std::is_same<T, bool>>>, bool> = true>
template <typename T, size_t H, size_t W>
class Matrix
{
	static_assert(std::is_arithmetic_v<T> && !std::is_same_v<T, bool>, "Type must be numeric & not bool!");
	static_assert(H >= 1, "Height must be greater than 0!");
	static_assert(W >= 1, "Width  must be greater than 0!");

	using row = std::array<T, W>;
	using this_t = Matrix<T, H, W>;
	using this_bool_t = BitMatrix<H, W>;
	using crd_t = MinUInt<std::max(H, W)>;
	using idx_t = MinUInt<H * W>;

	std::array<row, H> content;

public:
	constexpr Matrix() : content({}) {}
	~Matrix() {}

	constexpr row &operator[](size_t pos) const
	{
		return content[pos];
	}

	constexpr const T &operator()(size_t y, size_t x) const
	{
		return content[y][x];
	}

	T &operator()(size_t y, size_t x)
	{
		return content[y][x];
	}

	constexpr const T &operator()(size_t pos) const
	{
		return content[pos / W][pos % W];
	}

	T &operator()(size_t pos)
	{
		return content[pos / W][pos % W];
	}

	T &first()
	{
		return content[0][0];
	}

	template <typename Ta, size_t Ha, size_t Wa>
	friend BitMatrix<Ha, Wa> operator==(const Matrix<Ta, Ha, Wa> &A, Ta c)
	{
		BitMatrix<Ha, Wa> temp;
		for (size_t y = 0; y < H; ++y)
			for (size_t x = 0; x < W; ++x)
				temp(y, x) = A.content[y][x] == c;
		return temp;
	}

	template <typename Ta, size_t Ha, size_t Wa>
	friend BitMatrix<Ha, Wa> operator!=(const Matrix<Ta, Ha, Wa> &A, Ta c)
	{
		BitMatrix<Ha, Wa> temp;
		for (size_t y = 0; y < H; ++y)
			for (size_t x = 0; x < W; ++x)
				temp(y, x) = A.content[y][x] != c;
		return temp;
	}

	template <typename Ta, size_t Ha, size_t Wa>
	friend BitMatrix<Ha, Wa> operator<=(const Matrix<Ta, Ha, Wa> &A, Ta c)
	{
		BitMatrix<Ha, Wa> temp;
		for (size_t y = 0; y < H; ++y)
			for (size_t x = 0; x < W; ++x)
				temp(y, x) = A.content[y][x] <= c;
		return temp;
	}

	template <typename Ta, size_t Ha, size_t Wa>
	friend BitMatrix<Ha, Wa> operator>=(const Matrix<Ta, Ha, Wa> &A, Ta c)
	{
		BitMatrix<Ha, Wa> temp;
		for (size_t y = 0; y < H; ++y)
			for (size_t x = 0; x < W; ++x)
				temp(y, x) = A.content[y][x] >= c;
		return temp;
	}

	template <typename Ta, size_t Ha, size_t Wa>
	friend BitMatrix<Ha, Wa> operator<(const Matrix<Ta, Ha, Wa> &A, Ta c)
	{
		BitMatrix<Ha, Wa> temp;
		for (size_t y = 0; y < H; ++y)
			for (size_t x = 0; x < W; ++x)
				temp(y, x) = A.content[y][x] < c;
		return temp;
	}

	template <typename Ta, size_t Ha, size_t Wa>
	friend BitMatrix<Ha, Wa> operator>(const Matrix<Ta, Ha, Wa> &A, Ta c)
	{
		BitMatrix<Ha, Wa> temp;
		for (size_t y = 0; y < H; ++y)
			for (size_t x = 0; x < W; ++x)
				temp(y, x) = A.content[y][x] > c;
		return temp;
	}

	bool any() const
	{
		for (size_t y = 0; y < H; ++y)
			for (size_t x = 0; x < W; ++x)
				if (content[y][x])
					return true;
		return false;
	}

	bool all() const
	{
		for (size_t y = 0; y < H; ++y)
			for (size_t x = 0; x < W; ++x)
				if (!content[y][x])
					return false;
		return true;
	}

	bool none() const
	{
		for (size_t y = 0; y < H; ++y)
			for (size_t x = 0; x < W; ++x)
				if (content[y][x])
					return false;
		return true;
	}

	template <typename Ta, size_t Ha, size_t Wa>
	friend std::ostream &operator<<(std::ostream &os, const Matrix<Ta, Ha, Wa> &A)
	{
		for (size_t y = 0; y < H; ++y)
		{
			for (size_t x = 0; x < W; ++x)
				os << +A.content[y][x] << '\t';
			os << std::endl;
		}
		os << "Any: " << A.any() << ", All: " << A.all() << ", None: " << A.none() << std::endl
		   << std::endl;

		return os;
	}

	// ITERATORS, DONT INTERVENE

	typedef size_t size_type;

	class iterator
	{
	public:
		typedef iterator self_type;
		typedef T value_type;
		typedef T &reference;
		typedef T *pointer;
		typedef std::forward_iterator_tag iterator_category;
		typedef std::ptrdiff_t difference_type;
		iterator(pointer ptr) : ptr_(ptr) {}
		self_type operator++()
		{
			self_type i = *this;
			ptr_++;
			return i;
		}
		self_type operator++(int junk)
		{
			ptr_++;
			return *this;
		}
		reference operator*() { return *ptr_; }
		pointer operator->() { return ptr_; }
		bool operator==(const self_type &rhs) { return ptr_ == rhs.ptr_; }
		bool operator!=(const self_type &rhs) { return ptr_ != rhs.ptr_; }

	private:
		pointer ptr_;
	};

	class const_iterator
	{
	public:
		typedef const_iterator self_type;
		typedef T value_type;
		typedef T &reference;
		typedef T *pointer;
		typedef std::ptrdiff_t difference_type;
		typedef std::forward_iterator_tag iterator_category;
		const_iterator(pointer ptr) : ptr_(ptr) {}
		self_type operator++()
		{
			self_type i = *this;
			ptr_++;
			return i;
		}
		self_type operator++(int junk)
		{
			ptr_++;
			return *this;
		}
		const reference operator*() { return *ptr_; }
		const pointer operator->() { return ptr_; }
		bool operator==(const self_type &rhs) { return ptr_ == rhs.ptr_; }
		bool operator!=(const self_type &rhs) { return ptr_ != rhs.ptr_; }

	private:
		pointer ptr_;
	};

	iterator begin()
	{
		return iterator(&content[0][0]);
	}

	iterator end()
	{
		return iterator(&content[H - 1][W - 1] + 1);
	}

	const_iterator begin() const
	{
		return const_iterator(&content[0][0]);
	}

	const_iterator end() const
	{
		return const_iterator(&content[H - 1][W - 1] + 1);
	}
};

// common_type_t<int, float> xd = 5.0;