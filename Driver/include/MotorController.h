#pragma once

#include <esp_timer.h>

#include "ESP32Encoder.h"
#include "Motor.h"
#include "Controller.h"

class MotorController
{
	Motor *motor = nullptr;
	ESP32Encoder *encoder = nullptr;
	Controller<> *controller = nullptr;

	esp_timer_handle_t periodic_timer;

	int64_t time_last = 0;
	int64_t tick_last = 0;

	float setpoint = 0;
	float curr_speed = 0.0 / 0.0;

	float steering = 0;

	int i = 0;

public:
	MotorController(Motor * = nullptr, ESP32Encoder * = nullptr, Controller<> * = nullptr);
	~MotorController();

	void attachMotor(Motor * = nullptr);
	void attachEncoder(ESP32Encoder * = nullptr);
	void attachController(Controller<> * = nullptr);

	void setSetpoint(float);

	void start();
	void stop();

	bool is_on();

private:
	static void callback(void *);
};
