#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

// CPP headers
#include <string>

// ESP system
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <string.h>

// FreeRTOS
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// Memory
#include <esp_heap_caps.h>

// Other
#include "functions.h"

const char *TAG = "[" __TIME__
				  "]LnDtctr🅱 ";

extern "C" void app_main(void)
{
	for (int p : {12, 13, 14, 15})
	{
		PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[p], PIN_FUNC_GPIO);
		gpio_reset_pin(static_cast<gpio_num_t>(p));
	}

	gpio_reset_pin(GPIO_NUM_4);
	gpio_reset_pin(GPIO_NUM_33);
	gpio_set_direction(GPIO_NUM_4, GPIO_MODE_OUTPUT);
	gpio_set_direction(GPIO_NUM_33, GPIO_MODE_OUTPUT);

	gpio_set_level(GPIO_NUM_33, 0);

	esp_err_t cam = init_camera();
	if (cam != ESP_OK)
		return;

	sdmmc_card_t card = init_sd();
	if (!card.is_mem)
		return;

	size_t picnum = 0;

	double startTime;
	uint32_t startTick;

	gpio_set_level(GPIO_NUM_4, 0);

	while (1)
	{
		picnum++;
		startTick = esp_log_timestamp();
		startTime = GetTime();

		gpio_set_level(GPIO_NUM_33, 0);

		ESP_LOGI(TAG, "Taking picture...");

		camera_fb_t *pic = esp_camera_fb_get();

		std::string fname = (std::string)MOUNT_POINT + "/photo" + std::to_string(picnum) + ".jpg";

		FILE *fp = fopen(fname.c_str(), "w");
		if (fp != NULL)
		{
			fwrite(pic->buf, sizeof(uint8_t), pic->len, fp);
			fclose(fp);
		}
		ESP_LOGI(TAG, "Diff: %i ms, %lf ms", esp_log_timestamp() - startTick, GetTime() - startTime);

		esp_camera_fb_return(pic);

		vTaskDelay(pdMS_TO_TICKS(500));
		gpio_set_level(GPIO_NUM_33, 1);
		vTaskDelay(pdMS_TO_TICKS(500));
	}
	deinit_sd(card);

	return;
}
