#pragma once

#include "Mat22.h"
#include "Math.h"

#include "MotorController.h"

extern const char *TAG;

class Vehicle
{
public:
	using F = float;
	using V = Vec2<F>;
	using M = Mat22<F>;

	static constexpr F axis_radius = 80; // mm

	static constexpr M step2dist = M::unit() * Motor::encoder_resolution * Motor::wheel_radius;
	static constexpr M dist2dso = M(0.5, 0.5, 0.5 / axis_radius, -0.5 / axis_radius);

	static constexpr M step2dso = step2dist * dist2dso;
	static constexpr M dso2step = ~step2dso;

	V XY = V::null();
	F O = deg2rad(static_cast<F>(90));

	MotorController *MCL;
	MotorController *MCR;

public:
	Vehicle(MotorController *mcl, MotorController *mcr) : MCL(mcl), MCR(mcr) {}
	~Vehicle() {}

	void resetPos()
	{
		XY = V::null();
		O = deg2rad(static_cast<F>(90));
	}

	void move(const F dS, const F dO)
	{
		F argRot = O + dO * static_cast<F>(0.5);

		XY += M::rotYX(-argRot) * V(sinxox(dO), versinxox(dO)) * dS;
		O += dO;
	}

	void updateSetpoints(F dz, F dx)
	{
		F rds = dzx2rds(dz, dx);
		V vlr = rds2vlr(rds);

		ESP_LOGI(TAG, "Rds: %f; MtrL: %f, MtrR: %f", rds, vlr.a, vlr.b);

		MCL->setSetpoint(vlr.a * Motor::Vmax);
		MCR->setSetpoint(vlr.b * Motor::Vmax);
	}

	void start()
	{
		MCL->start();
		MCR->start();
	}
	void stop()
	{
		MCL->stop();
		MCR->stop();
	}

	// ======{ STATICS }======

	inline static constexpr V dlr2dso(V dLR)
	{
		return step2dso * dLR;
	}
	inline static constexpr V dso2dlr(V dSO)
	{
		return dso2step * dSO;
	}

	static constexpr V rds2vlr(F turn_radius) // convert radius to velocities
	{
		if (std::isnan(turn_radius))
			return {0, 0};
		if (std::isinf(turn_radius))
			return {1, 1};

		if (turn_radius > 0)
			return {1, (turn_radius - axis_radius) / (turn_radius + axis_radius)};
		if (turn_radius < 0)
			return {(turn_radius + axis_radius) / (turn_radius - axis_radius), 1};

		return {1, -1}; // turn right when exactly 180deg, should never happen lol
	}

	static F dzx2rds(F dz, F dx) // convert dz dx to appropriate turning radius
	{
		static constexpr F k = 1.0f / 2;

		if ((dz == 0 && dx == 0) || std::abs(dz) > 200.f || std::abs(dx) > 200.f)
			return 0.0 / 0.0;

		F ln = std::max(k * dz, 0.0f);
		F rm = dz - ln;
		return ln * (rm + std::sqrt(dx * dx + rm * rm)) / dx;
	}
};
