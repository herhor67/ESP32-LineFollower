#pragma once

#include <limits>
#include <initializer_list>

template <size_t pos, typename T>
const T &get(const std::initializer_list<T> &il)
{
	return il.begin()[pos];
}

template <typename fp = float>
class AntiWindUp
{
	using nl = std::numeric_limits<fp>;
	// protected:
public:
	fp O, Omax, Omin;

public:
	AntiWindUp(fp o = 0, fp omax = nl::max(), fp omin = nl::lowest()) : O(o), Omax(omax), Omin(omin) {}
	virtual ~AntiWindUp() {}

	void setOmax(fp omax = nl::max())
	{
		Omax = omax;
		bound();
	}
	void setOmin(fp omin = nl::lowest())
	{
		Omin = omin;
		bound();
	}

	void reset()
	{
		O = 0;
		bound();
	}

	virtual fp updateO(const std::initializer_list<fp> &Is, fp dt) = 0;

	operator fp() const
	{
		return O;
	}

protected:
	fp bound()
	{
		if (O < Omin)
			O = Omin;
		else if (O > Omax)
			O = Omax;

		return O;
	}
};

template <typename fp = float>
class Integral : public AntiWindUp<fp>
{
	using AW = AntiWindUp<fp>;
	using nl = std::numeric_limits<fp>;

public:
	using AW::AW;
	~Integral() {}

	fp updateO(const std::initializer_list<fp> &Is, fp dt)
	{
		// static_assert(Is.size() == 1, "Is must be of size 1, I (input)!");
		fp I = get<0>(Is);
		AW::O += I * dt;

		return AW::bound();
	}
};

template <typename fp = float>
class Differential : public AntiWindUp<fp>
{
	using AW = AntiWindUp<fp>;
	using nl = std::numeric_limits<fp>;
	fp prevI = 0;

public:
	using AW::AW;
	~Differential() {}

	void reset()
	{
		prevI = 0;
		AW::reset();
	}

	fp updateO(const std::initializer_list<fp> &Is, fp dt)
	{
		// static_assert(Is.size() == 1, "Is must be of size 1, I (input)!");
		fp I = get<0>(Is);

		AW::O = (I - prevI) / dt;
		prevI = I;

		return AW::bound();
	}
};

template <typename fp = float>
class Proportional : public AntiWindUp<fp>
{
	using AW = AntiWindUp<fp>;
	using nl = std::numeric_limits<fp>;

public:
	using AW::AW;
	~Proportional() {}

	fp updateO(const std::initializer_list<fp> &Is, fp dt)
	{
		// static_assert(Is.size() == 1, "Is must be of size 1, I (input)!");
		fp I = get<0>(Is);

		AW::O = I;

		return AW::bound();
	}
};

template <typename fp = float>
class Controller : public AntiWindUp<fp>
{
	using AW = AntiWindUp<fp>;
	using nl = std::numeric_limits<fp>;
	// protected:
public:
	fp Kp, Ki, Kd;
	bool useTs = false;

public:
	Proportional<fp> P;
	Integral<fp> I;
	Differential<fp> D;

	Controller(fp kp = 0, fp ki = 0, fp kd = 0, fp o = 0, fp omax = nl::max(), fp omin = nl::lowest()) : AW(o, omax, omin), Kp(kp), Ki(ki), Kd(kd) {}
	virtual ~Controller() {}

	virtual fp updateO(const std::initializer_list<fp> &Is = {}, fp dt = 0)
	{
		if (!useTs)
			AW::O = Kp * P + Ki * I + Kd * D;
		else // Ts
			AW::O = Kp * (P + dt / Ki * I + Kd * dt * D);

		return bound();
	}

	void setKs(fp kp, fp ki, fp kd)
	{
		useTs = false;
		Kp = kp;
		Ki = ki;
		Kd = kd;
	}
	void setTs(fp kp, fp ti, fp td)
	{
		useTs = true;
		Kp = kp;
		Ki = ti;
		Kd = td;
	}

protected:
	fp bound()
	{
		return AW::bound();
	}
};

template <typename fp = float>
class PID__Controller : public Controller<fp>
{
	using C = Controller<fp>;
	using nl = std::numeric_limits<fp>;

public:
	using C::C;
	~PID__Controller() {}

	fp updateO(const std::initializer_list<fp> &Is, fp dt)
	{
		// static_assert(Is.size() == 2, "Is must be of size 2, W (setpoint) and Y (plant output)!");
		fp W = get<0>(Is);
		fp Y = get<1>(Is);

		C::P.updateO({W - Y}, dt);
		C::I.updateO({W - Y}, dt);
		C::D.updateO({W - Y}, dt);

		return C::updateO({}, dt);
	}
};

template <typename fp = float>
class PI_D_Controller : public Controller<fp>
{
	using C = Controller<fp>;
	using nl = std::numeric_limits<fp>;

public:
	using C::C;
	~PI_D_Controller() {}

	fp updateO(const std::initializer_list<fp> &Is, fp dt)
	{
		// static_assert(Is.size() == 2, "Is must be of size 2, W (setpoint) and Y (plant output)!");
		fp W = get<0>(Is);
		fp Y = get<1>(Is);

		C::P.updateO({W - Y}, dt);
		C::I.updateO({W - Y}, dt);
		C::D.updateO({-Y}, dt);

		return C::updateO({}, dt);
	}
};

template <typename fp = float>
class I_PD_Controller : public Controller<fp>
{
	using C = Controller<fp>;
	using nl = std::numeric_limits<fp>;

public:
	using C::C;
	~I_PD_Controller() {}

	fp updateO(const std::initializer_list<fp> &Is, fp dt)
	{
		// static_assert(Is.size() == 2, "Is must be of size 2, W (setpoint) and Y (plant output)!");
		fp W = get<0>(Is);
		fp Y = get<1>(Is);

		C::I.updateO({W - Y}, dt);
		C::P.updateO({-Y}, dt);
		C::D.updateO({-Y}, dt);

		return C::updateO({}, dt);
	}
};

template <typename fp = float>
class PI__Controller : public Controller<fp>
{
	using C = Controller<fp>;
	using nl = std::numeric_limits<fp>;

public:
	using C::C;
	~PI__Controller() {}

	fp updateO(const std::initializer_list<fp> &Is, fp dt)
	{
		// static_assert(Is.size() == 2, "Is must be of size 2, W (setpoint) and Y (plant output)!");
		fp W = get<0>(Is);
		fp Y = get<1>(Is);

		C::P.updateO({W - Y}, dt);
		C::I.updateO({W - Y}, dt);

		return C::updateO({}, dt);
	}
};

//