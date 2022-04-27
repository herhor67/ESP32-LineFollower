#pragma once

#include "Math.h"

class Motor
{
public:
	using F = float;
	using I = int;
	friend class Vehicle;

	static constexpr F wheel_radius = 2;								  // cm
	static constexpr F encoder_resolution = deg2rad(static_cast<F>(0.5)); // rad per tick

	//	static constexpr I encoderDecode[] = { 0, -1,  1,  X,  1,  0,  X, -1, -1,  X,  0,  1,  X,  1, -1,  0 };
	static constexpr I encoderDecode[] = {0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0};

	I ticks = 0;
	uint8_t states = 0;

	constexpr Motor(bool a = 0, bool b = 0) : states(a << 1u | static_cast<int>(b)) {}
	~Motor() {}

	I encoderUpdate(bool a, bool b)
	{
		states <<= 2u;
		states |= a << 1u | static_cast<int>(b);

		return ticks += encoderDecode[states & 0b1111u];
	}
};
