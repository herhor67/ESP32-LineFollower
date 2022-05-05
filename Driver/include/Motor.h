#pragma once

#include "driver/mcpwm.h"
#include "soc/mcpwm_periph.h"

#include "Math.h"

class Motor
{
public:
	using F = float;
	using I = int;
	friend class Vehicle;

	static constexpr F wheel_radius = 2;								  // cm
	static constexpr F encoder_resolution = deg2rad(static_cast<F>(0.5)); // 720/rot = 1/0.5deg = ???/rad

	mcpwm_unit_t  mcpwm_num = MCPWM_UNIT_0;
	mcpwm_timer_t timer_num = MCPWM_TIMER_0;
	mcpwm_io_signals_t signalA = MCPWM0A;
	mcpwm_io_signals_t signalB = MCPWM0B;
	gpio_num_t pinA = GPIO_NUM_NC;
	gpio_num_t pinB = GPIO_NUM_NC;
	mcpwm_config_t pwm_config = {};
	uint32_t frequency = 1000;

	constexpr Motor(gpio_num_t pinA, gpio_num_t pinB) : pinA(pinA), pinB(pinB) {}
	~Motor() {}

	constexpr void setup(mcpwm_unit_t m_n, mcpwm_timer_t t_n, mcpwm_io_signals_t sA, mcpwm_io_signals_t sB, uint32_t f)
	{
		mcpwm_num = m_n;
		timer_num = t_n;
		signalA = sA;
		signalB = sB;
		frequency = f;
	}

	void initialize()
	{
		mcpwm_gpio_init(mcpwm_num, signalA, pinA);
		mcpwm_gpio_init(mcpwm_num, signalB, pinB);

		pwm_config.frequency = frequency;
		pwm_config.cmpr_a = 0;
		pwm_config.cmpr_b = 0;
		pwm_config.counter_mode = MCPWM_UP_COUNTER;
		pwm_config.duty_mode = MCPWM_DUTY_MODE_0;
		mcpwm_init(mcpwm_num, timer_num, &pwm_config);
	}


	void motor_duty(float duty_cycle)
	{
		if (duty_cycle > 0)
			motor_forward(duty_cycle);
		else if (duty_cycle < 0)
			motor_backward(-duty_cycle);
		else
			motor_stop();
	}

	void motor_stop()
	{
		mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_GEN_A);
		mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_GEN_B);
	}

private:
	void motor_forward(float duty_cycle)
	{
		if (duty_cycle > 100.0f)
			duty_cycle = 100.0f;
		
		mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_GEN_B);
		mcpwm_set_duty      (mcpwm_num, timer_num, MCPWM_GEN_A, duty_cycle);
		mcpwm_set_duty_type (mcpwm_num, timer_num, MCPWM_GEN_A, MCPWM_DUTY_MODE_0);
	}

	void motor_backward(float duty_cycle)
	{
		if (duty_cycle > 100.0f)
			duty_cycle = 100.0f;
		
		mcpwm_set_signal_low(mcpwm_num, timer_num, MCPWM_GEN_A);
		mcpwm_set_duty      (mcpwm_num, timer_num, MCPWM_GEN_B, duty_cycle);
		mcpwm_set_duty_type (mcpwm_num, timer_num, MCPWM_GEN_B, MCPWM_DUTY_MODE_0);
	}

};
