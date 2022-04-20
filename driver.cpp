
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

// CPP headers
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

// Memory
#include <esp_heap_caps.h>

static const char *TAG = "Cntrllr🅱 ";

#include <esp_timer.h>
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

	// gpio_config(&gpio4_conf);

	double startTime;
	uint32_t startTick;

	gpio_set_direction(GPIO_NUM_13, GPIO_MODE_OUTPUT);
	gpio_set_direction(GPIO_NUM_12, GPIO_MODE_OUTPUT);

	while (1)
	{

		startTick = esp_log_timestamp();
		startTime = GetTime();

		gpio_set_level(GPIO_NUM_13, 0);
		gpio_set_level(GPIO_NUM_12, 1);

		vTaskDelay(pdMS_TO_TICKS(10));

		gpio_set_level(GPIO_NUM_13, 1);
		gpio_set_level(GPIO_NUM_12, 0);

		vTaskDelay(pdMS_TO_TICKS(10));

		// ESP_LOGI(TAG, "Diff: %i ms, %lf ms", esp_log_timestamp() - startTick, GetTime() - startTime);

		// ESP_LOGI(TAG, "Diff: %lf ms", GetTime() - startTime);

		// vTaskDelay(pdMS_TO_TICKS(1000));
	}

	return;
}
