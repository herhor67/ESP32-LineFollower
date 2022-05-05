
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

// CPP headers
#include <inttypes.h>
#include <string>

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
#include <esp_intr_alloc.h>

// Memory
#include <esp_heap_caps.h>
#include <esp_timer.h>

// Classes
#include "ESP32Encoder.h"
#include "Motor.h"
#include "Controller.h"

#include "Buzzer.h"
//#include "Vehicle.h"
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
Buzzer::note_t melody[] = {
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
	Note::G5};

// note durations: 4 = quarter note, 8 = eighth note, etc, also called tempo:
Buzzer::duration_t noteDurations[] = {
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
	Duration::o4};

static const char *TAG = "[" __TIME__
						 "]Cntrllr🅱 ";

double GetTime()
{
	return (double)esp_timer_get_time() / 1000;
}

#ifdef __cplusplus
extern "C"
#endif
	void
	app_main(void)
{

	ESP_LOGI(TAG, "Wstało!");

	PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[12], PIN_FUNC_GPIO);
	PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[13], PIN_FUNC_GPIO);
	PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[14], PIN_FUNC_GPIO);
	PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[15], PIN_FUNC_GPIO);

	double startTime;
	uint32_t startTick;

	// gpio_reset_pin(GPIO_NUM_12);
	// gpio_reset_pin(GPIO_NUM_13);

	// gpio_set_direction(GPIO_NUM_12, GPIO_MODE_OUTPUT);
	// gpio_set_direction(GPIO_NUM_13, GPIO_MODE_OUTPUT);

	// gpio_reset_pin(GPIO_NUM_34);
	// gpio_reset_pin(GPIO_NUM_35);
	// gpio_set_direction(GPIO_NUM_34, GPIO_MODE_INPUT);
	// gpio_set_direction(GPIO_NUM_35, GPIO_MODE_INPUT);
	// gpio_pullup_dis(GPIO_NUM_34);
	// gpio_pullup_dis(GPIO_NUM_35);

	// gpio_install_isr_service(ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_LEVEL3); // ESP_INTR_FLAG_EDGE

	// gpio_set_intr_type(GPIO_NUM_34, GPIO_INTR_ANYEDGE);
	// gpio_set_intr_type(GPIO_NUM_35, GPIO_INTR_ANYEDGE);

	// gpio_isr_handler_add(GPIO_NUM_34, interrupt_handler, (void *)GPIO_NUM_34);
	// gpio_isr_handler_add(GPIO_NUM_35, interrupt_handler, (void *)GPIO_NUM_35);

	// ESP32Encoder encoderL;
	// ESP32Encoder encoderR;

	// encoderR.attachFullQuad(GPIO_NUM_34, GPIO_NUM_35);
	// encoderL.attachFullQuad(GPIO_NUM_36, GPIO_NUM_39);

	// encoderL.clearCount();
	// encoderR.clearCount();

	// Motor motorL(GPIO_NUM_12, GPIO_NUM_13);
	// motorL.initialize();

	// Controller ctrlL(&motorL, &encoderL);

	// ctrlL.start();

	Buzzer bzr(LEDC_TIMER_1, LEDC_CHANNEL_0, GPIO_NUM_14);

	bzr.init();

	bzr.loadSong(&melody[0], &noteDurations[0], sizeof(melody) / sizeof(melody[0]));

	bzr.play(true);

	bool xd = false;

	while (1)
	{
		startTick = esp_log_timestamp();
		startTime = GetTime();

		// int a = 400;
		// float PWM = fabs(((int)startTick / 10) % a - a / 2) - a / 4;
		// motorL.motor_duty(PWM);

		// gpio_set_level(GPIO_NUM_12, 0);
		// gpio_set_level(GPIO_NUM_13, 1);

		// vTaskDelay(pdMS_TO_TICKS(1000));

		// gpio_set_level(GPIO_NUM_12, 1);
		// gpio_set_level(GPIO_NUM_13, 0);

		// vTaskDelay(pdMS_TO_TICKS(1000));

		// ESP_LOGI(TAG, "PWM: %f ; EncoderL: %" PRId64 " ; EncoderR: %" PRId64, PWM, encoderL.getCount(), encoderR.getCount());
		// vTaskDelay(pdMS_TO_TICKS(50));

		ESP_LOGI(TAG, "Diff: %i ms, %lf ms", esp_log_timestamp() - startTick, GetTime() - startTime);

		if (xd)
			bzr.pause();
		else
			bzr.resume();

		xd = !xd;

		vTaskDelay(pdMS_TO_TICKS(1000));
	}

	return;
}
