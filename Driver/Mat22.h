#pragma once

#include <ostream>

#include "gcem.hpp"

template <typename T>
class Vec2
{
public:
	// [ a ]
	// [ b ]
	T a;
	T b;

	constexpr Vec2(T a = 0, T b = 0) : a(a), b(b) {}
	~Vec2() {}

	constexpr Vec2<T> &operator+=(const Vec2<T> &r)
	{
		a += r.a;
		b += r.b;
		return *this;
	}
	constexpr Vec2<T> &operator-=(const Vec2<T> &r)
	{
		a -= r.a;
		b -= r.b;
		return *this;
	}
	constexpr Vec2<T> &operator*=(const T &r)
	{
		a *= r;
		b *= r;
		return *this;
	}
	constexpr Vec2<T> &operator/=(const T &r)
	{
		a /= r;
		b /= r;
		return *this;
	}

	static constexpr Vec2 unit()
	{
		return Vec2(1, 1);
	}
	static constexpr Vec2 null()
	{
		return Vec2(0, 0);
	}
};

template <typename T>
constexpr Vec2<T> operator+(Vec2<T> l, const Vec2<T> &r)
{
	return l += r;
}
template <typename T>
constexpr Vec2<T> operator-(Vec2<T> l, const Vec2<T> &r)
{
	return l -= r;
}

template <typename T>
constexpr Vec2<T> operator*(Vec2<T> l, const T &r)
{
	return l *= r;
}
template <typename T>
constexpr Vec2<T> operator/(Vec2<T> l, const T &r)
{
	return l /= r;
}

template <typename T>
std::ostream &operator<<(std::ostream &os, const Vec2<T> &m)
{
	os << "\n[ " << m.a << " ]\n[ " << m.c << " ]" << std::endl;
	return os;
}

template <typename T>
class Mat22
{
public:
	// [ a | b ]
	// [ --+-- ]
	// [ c | d ]
	T a;
	T b;
	T c;
	T d;

	constexpr Mat22(T a = 0, T b = 0, T c = 0, T d = 0) : a(a), b(b), c(c), d(d) {}
	~Mat22() = default;

	constexpr Mat22<T> &operator+=(const Mat22<T> &r)
	{
		a += r.a;
		b += r.b;
		c += r.c;
		d += r.d;
		return *this;
	}
	constexpr Mat22<T> &operator-=(const Mat22<T> &r)
	{
		a -= r.a;
		b -= r.b;
		c -= r.c;
		d -= r.d;
		return *this;
	}
	constexpr Mat22<T> &operator*=(const Mat22<T> &r)
	{
		T t;

		t = a * r.a + b * r.c;
		b = a * r.b + b * r.d;
		a = t;

		t = c * r.a + d * r.c;
		d = c * r.b + d * r.d;
		c = t;

		return *this;
	}

	constexpr Mat22<T> &operator*=(const T &r)
	{
		a *= r;
		b *= r;
		c *= r;
		d *= r;
		return *this;
	}
	constexpr Mat22<T> &operator/=(const T &r)
	{
		a /= r;
		b /= r;
		c /= r;
		d /= r;
		return *this;
	}

	constexpr Mat22<T> operator-() const
	{
		return null() -= *this;
	}
	constexpr Mat22<T> operator~() const
	{
		Mat22<T> temp(null());
		temp.a = d;
		temp.d = a;
		temp.b = -b;
		temp.c = -c;

		temp /= *(*this);

		return temp;
	}

	constexpr T operator*() const
	{
		return a * d - b * c;
	}

	static constexpr Mat22 unit()
	{
		return Mat22(1, 0, 0, 1);
	}
	static constexpr Mat22 null()
	{
		return Mat22(0, 0, 0, 0);
	}
	static constexpr Mat22 rotXY(T angle)
	{
		T sin = gcem::sin(angle);
		T cos = gcem::cos(angle);
		return Mat22(cos, -sin, sin, cos);
	}
	static constexpr Mat22 rotYX(T angle)
	{
		T sin = gcem::sin(angle);
		T cos = gcem::cos(angle);
		return Mat22(-sin, cos, cos, sin);
	}
	constexpr Mat22 flipAB()
	{
		return Mat22(b, a, d, c);
	}
	constexpr Mat22 flipAC()
	{
		return Mat22(c, d, a, b);
	}
};

template <typename T>
constexpr Mat22<T> operator+(Mat22<T> l, const Mat22<T> &r)
{
	return l += r;
}
template <typename T>
constexpr Mat22<T> operator-(Mat22<T> l, const Mat22<T> &r)
{
	return l -= r;
}
template <typename T>
constexpr Mat22<T> operator*(Mat22<T> l, const Mat22<T> &r)
{
	return l *= r;
}

template <typename T>
constexpr Mat22<T> operator*(Mat22<T> l, const T &r)
{
	return l *= r;
}
template <typename T>
constexpr Mat22<T> operator/(Mat22<T> l, const T &r)
{
	return l /= r;
}

template <typename T>
std::ostream &operator<<(std::ostream &os, const Mat22<T> &m)
{
	os << "\n[ " << m.a << " | " << m.b << " ]\n[ --+-- ]\n[ " << m.c << " | " << m.d << " ]" << std::endl;
	return os;
}

template <typename T>
constexpr Vec2<T> &operator*=(Vec2<T> &v, const Mat22<T> &m)
{
	T t = m.a * v.a + m.b * v.b;
	v.b = m.c * v.a + m.d * v.b;
	v.a = t;
	return v;
}

template <typename T>
constexpr Vec2<T> operator*(const Mat22<T> &m, Vec2<T> v)
{
	return v *= m;
}
