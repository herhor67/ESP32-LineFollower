#pragma once

#include "Mat22.h"
#include "Math.h"

#include "Motor.h"

class Vehicle
{
public:
	using F = float;
	using V = Vec2<F>;
	using M = Mat22<F>;

	static constexpr F turn_radius = 10; // cm

	static constexpr M step2dist = M::unit() * Motor::encoder_resolution * Motor::wheel_radius;
	static constexpr M dist2dso = M(0.5, 0.5, 1 / turn_radius, -1 / turn_radius);

	static constexpr M step2dso = step2dist * dist2dso;
	static constexpr M dso2step = ~step2dso;

	V XY = V::null();
	F O = deg2rad(static_cast<F>(90));

	Motor mtrL, mtrR;

public:
	Vehicle() {}
	~Vehicle() = default;

	// void reset()
	//{
	//	XY = V::null();
	//	O  = deg2rad(static_cast<F>(90));
	// }

	// void move(const V& dXY, const F dO)
	//{
	//	XY += dXY;
	//	O  += dO;
	// }

	void move(const F dS, const F dO)
	{
		F argRot = O + dO * static_cast<F>(0.5);

		XY += M::rotYX(-argRot) * V(sinxox(dO), versinxox(dO)) * dS;
		O += dO;
	}

	static constexpr V dlr2dso(V dLR)
	{
		return step2dso * dLR;
	}
	static constexpr V dso2dlr(V dSO)
	{
		return dso2step * dSO;
	}
};

// function[dS, dO] = lr2so(dL, dR)
// global Matrix_lr2soF
// dLR = [dL; dR];
//
// dSO = Matrix_lr2soF * dLR;
//
// dS = dSO(1);
// dO = dSO(2);
// end

// function[dL, dR] = so2lr(dS, dO)
// global Matrix_so2lrF
// dSO = [dS; dO];
//
// dLR = Matrix_so2lrF * dSO;
//
// dL = dLR(1);
// dR = dLR(2);
// end
