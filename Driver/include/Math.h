#pragma once

#include <array>
#include <cmath>

#include "gcem.hpp"

#include "Mat22.h"

template <typename T>
using Point = Vec2<T>;

template <typename T>
int sgn(T val)
{
	return (T(0) < val) - (val < T(0));
}

template <size_t count, typename T>
constexpr std::array<T, count + 1> interpolate(T v1, T v2)
{
	std::array<T, count + 1> temp;

	T diff = v2 - v1;

	for (size_t i = 0; i < count; ++i)
	{
		temp[i] = v1 + diff * i / count;
	}
	temp[count] = v2;

	return temp;
}

template <typename T>
constexpr T deg2rad(T degrees)
{
	constexpr T radperdeg = static_cast<T>(3.14159265358979323846 / 180.0);
	return degrees * radperdeg;
}

template <typename T>
constexpr T sinxox(T x)
{
	if (x == static_cast<T>(0))
		return static_cast<T>(1);

	return gcem::sin(x) / x;
}

template <typename T>
constexpr T versinxox(T x)
{
	if (x == static_cast<T>(0))
		return x;

	return (static_cast<T>(1) - gcem::cos(x)) / x;
}

template <size_t count, typename T>
std::array<Point<T>, count + 1> quadBezier(Point<T> p0, Point<T> p1, Point<T> p2)
{
	constexpr auto timePoints = interpolate(static_cast<T>(0), static_cast<T>(1), count);

	std::array<Point<T>, count> temp;

	for (size_t i = 0; i <= count; ++i)
	{
		T t = timePoints[i];
		T u = static_cast<T>(1) - t;

		temp[i] = p0 * (u * u) + p1 * (u * t) + p2 * (t * t);
	}

	return temp;
}
