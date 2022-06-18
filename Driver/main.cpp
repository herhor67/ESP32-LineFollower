
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

// CPP headers
#include <inttypes.h>
#include <string>
#include <iostream>

// ESP system
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <string.h>

#include <driver/gpio.h>

// FreeRTOS
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// Interrupts
//#include <esp_intr_alloc.h>

// Memory
//#include <esp_heap_caps.h>
#include <esp_timer.h>

// Classes
// #include "ESP32Encoder.h"
// #include "Motor.h"
//#include "MotorController.h"
#include "Vehicle.h"

#include "Buzzer.h"
#include "functions.h"

//
//

// static const musical_buzzer_notation_t notation[] = {
// 	{740, 400},
// 	{740, 600},
// 	{784, 400},
// 	{880, 400},
// 	{880, 400},
// 	{784, 400},
// 	{740, 400},
// 	{659, 400},
// 	{587, 400},
// 	{587, 400},
// 	{659, 400},
// 	{740, 400},
// 	{740, 400},
// 	{740, 200},
// 	{659, 200},
// 	{659, 800},
// 	{740, 400},
// 	{740, 600},
// 	{784, 400},
// 	{880, 400},
// 	{880, 400},
// 	{784, 400},
// 	{740, 400},
// 	{659, 400},
// 	{587, 400},
// 	{587, 400},
// 	{659, 400},
// 	{740, 400},
// 	{659, 400},
// 	{659, 200},
// 	{587, 200},
// 	{587, 800},
// 	{659, 400},
// 	{659, 400},
// 	{740, 400},
// 	{587, 400},
// 	{659, 400},
// 	{740, 200},
// 	{784, 200},
// 	{740, 400},
// 	{587, 400},
// 	{659, 400},
// 	{740, 200},
// 	{784, 200},
// 	{740, 400},
// 	{659, 400},
// 	{587, 400},
// 	{659, 400},
// 	{440, 400},
// 	{440, 400},
// 	{740, 400},
// 	{740, 600},
// 	{784, 400},
// 	{880, 400},
// 	{880, 400},
// 	{784, 400},
// 	{740, 400},
// 	{659, 400},
// 	{587, 400},
// 	{587, 400},
// 	{659, 400},
// 	{740, 400},
// 	{659, 400},
// 	{659, 200},
// 	{587, 200},
// 	{587, 800},
// };

// notes in the melody:
note_t jnglBlls_nts[] = {
	Note::E5,
	Note::E5,
	Note::E5,
	Note::E5,
	Note::E5,
	Note::E5,
	Note::E5,
	Note::G5,
	Note::C5,
	Note::D5,
	Note::E5,
	Note::F5,
	Note::F5,
	Note::F5,
	Note::F5,
	Note::F5,
	Note::E5,
	Note::E5,
	Note::E5,
	Note::E5,
	Note::E5,
	Note::D5,
	Note::D5,
	Note::E5,
	Note::D5,
	Note::G5,
};

// note durations: 4 = quarter note, 8 = eighth note, etc, also called tempo:
duration_t jnglBlls_drtn[] = {
	Duration::o8,
	Duration::o8,
	Duration::o4,
	Duration::o8,
	Duration::o8,
	Duration::o4,
	Duration::o8,
	Duration::o8,
	Duration::o8,
	Duration::o8,
	Duration::o2,
	Duration::o8,
	Duration::o8,
	Duration::o8,
	Duration::o8,
	Duration::o8,
	Duration::o8,
	Duration::o8,
	Duration::o16,
	Duration::o16,
	Duration::o8,
	Duration::o8,
	Duration::o8,
	Duration::o8,
	Duration::o4,
	Duration::o4,
};

note_t stlphn_nts[] = {
	ST75,
	ST9,
	ST75,
	ST9,
	ST9,
	ST75,
	ST65,
	ST85,
	ST65,
	ST5,
};

duration_t stlphn_drtn[] = {
	Duration::o8,
	Duration::o16,
	Duration::o16,
	Duration::o16,
	Duration::o16,
	Duration::o8,
	Duration::o8,
	Duration::o16,
	Duration::o16,
	Duration::o8,
};

const char *TAG = "[" __TIME__
				  "]Cntrllr🅱 ";

// =========================

extern "C" void app_main(void)
{

	ESP_LOGI(TAG, "Wstało!");

	for (int p : {12, 13, 14, 15})
	{
		PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[p], PIN_FUNC_GPIO);
		gpio_reset_pin(static_cast<gpio_num_t>(p));
	}

	esp_err_t i2c = init_i2c();
	if (i2c != ESP_OK)
		return;

	uint32_t startTick = esp_log_timestamp();
	double startTime = GetTime();

	ESP32Encoder encoderL;
	ESP32Encoder encoderR;
	encoderL.attachFullQuad(GPIO_NUM_36, GPIO_NUM_39);
	encoderR.attachFullQuad(GPIO_NUM_34, GPIO_NUM_35);
	encoderL.clearCount();
	encoderR.clearCount();

	Motor motorL(GPIO_NUM_27, GPIO_NUM_26);
	Motor motorR(GPIO_NUM_25, GPIO_NUM_33);
	motorL.setup(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM0A, MCPWM0B, 10000);
	motorL.setup(MCPWM_UNIT_0, MCPWM_TIMER_1, MCPWM1A, MCPWM1B, 10000);
	motorL.initialize();
	motorR.initialize();

	PI__Controller<float> controllerL;
	controllerL.setTs(1, 0.03, 0);
	controllerL.setOmax(100);
	controllerL.setOmin(-100);
	controllerL.P.setOmax(50);
	controllerL.P.setOmin(-50);
	controllerL.I.setOmax(1000);
	controllerL.I.setOmin(-1000);

	PI__Controller<float> controllerR = controllerL;

	uint8_t mess[2 * sizeof(int)] = {0};
	uint8_t *messbyte = &mess[0];
	int *messint = reinterpret_cast<int *>(messbyte);

	MotorController MotCtrlL(&motorL, &encoderL, &controllerL);
	MotorController MotCtrlR(&motorR, &encoderR, &controllerR);

	Vehicle vehicle(&MotCtrlL, &MotCtrlR);

	// Buzzer bzr(GPIO_NUM_14);
	//
	// bzr.setTempo(72, Duration::o4);
	// bzr.loadSong(&stlphn_nts[0], &stlphn_drtn[0], 10); // sizeof(melody) / sizeof(melody[0])
	// bzr.start(false);

	vehicle.start();

	while (1)
	{
		int res = i2c_slave_read_buffer(I2C_NUM_0, messbyte, 15, 0);

		if (res > 0)
		{
			int dz = messint[0];
			int dx = messint[1];
			std::cout << dz << '\t' << dx << std::endl;

			vehicle.updateSetpoints(dz, dx);
		}

		//	vehicle.updateSetpoints(100, 1000);

		// vTaskDelay(1);

		// ESP_LOGI(TAG, "PWM: %f ; EncoderL: %" PRId64 " ; EncoderR: %" PRId64, PWM, encoderL.getCount(), encoderR.getCount());

		// ESP_LOGI(TAG, "Diff: %i ms, %lf ms", esp_log_timestamp() - startTick, GetTime() - startTime);
		// startTick = esp_log_timestamp();
		// startTime = GetTime();
		// vTaskDelay(pdMS_TO_TICKS(1000));
	}

	return;
}