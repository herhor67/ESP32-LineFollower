#pragma once

#include <esp_timer.h>

#include "ESP32Encoder.h"
#include "Motor.h"

class Controller
{
	Motor *motor = nullptr;
	ESP32Encoder *encoder = nullptr;

	esp_timer_handle_t periodic_timer;

	int64_t last = 0;

public:
	Controller(Motor * = nullptr, ESP32Encoder * = nullptr);
	~Controller();

	void attachMotor(Motor * = nullptr);
	void attachEncoder(ESP32Encoder * = nullptr);

	void start();
	void stop();

private:
	static void callback(void *);
};
