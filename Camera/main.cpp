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

// Image
#include "jpg2bin.h"
#include "BitMatrix.h"

// Other
#include "functions.h"

const char *TAG = "[" __TIME__
				  "]LnDtctr🅱 ";

constexpr auto vw_mdl = model_generator(150, 15, 66);
constexpr auto px_crds = pixel_generate_pos();
constexpr auto px_prms = pixel_generate_params(px_crds, vw_mdl);

// constexpr int a;
// constexpr float b;
// constexpr char c;
// constexpr std::tie(a, b, c) = make_tuple(123, 42.0, '7');

// void pic_take(void *pvParameter)
//{
//  vTaskDelete(NULL);
//}

extern "C" void app_main(void)
{

	// for (size_t i = 0; i < px_crds.size(); ++i)
	// 	ESP_LOGI(TAG, "%i\t%i\t%f\t%f\t%f", int(px_crds[i].first), int(px_crds[i].second), std::get<0>(px_prms[i]), std::get<1>(px_prms[i]), std::get<2>(px_prms[i]));

	// return;

	nvs_flash_init();

	// 0-5, 12-15, but lessgo everything
	for (int p : {12, 13, 14, 15})
	{
		PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[p], PIN_FUNC_GPIO);
		gpio_reset_pin(static_cast<gpio_num_t>(p));
	}

	esp_err_t cam = init_camera();
	if (cam != ESP_OK)
		return;

	// sdmmc_card_t card = init_sd();
	// if (!card.is_mem)
	// 	return;

	esp_err_t i2c = init_i2c();
	if (i2c != ESP_OK)
		return;

	gpio_set_direction(GPIO_NUM_4, GPIO_MODE_OUTPUT);
	gpio_set_direction(GPIO_NUM_33, GPIO_MODE_OUTPUT);

	//	TaskHandle_t pic_take_handle = NULL;
	//	xTaskCreatePinnedToCore(pic_take, "pic_take", 20000, nullptr, 10, &pic_take_handle, 0); // configMINIMAL_STACK_SIZE

	//	return;

	uint32_t startTick = esp_log_timestamp();
	double startTime = GetTime();

	gpio_set_level(GPIO_NUM_4, 1);
	// gpio_set_level(GPIO_NUM_4, 0);

	// MALLOC_CAP_INTERNAL // MALLOC_CAP_SPIRAM
	rgb_img_t &img = *(rgb_img_t *)heap_caps_calloc(1, sizeof(rgb_img_t), MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);
	bin_img_t &map = *(bin_img_t *)heap_caps_calloc(1, sizeof(bin_img_t), MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);

	double tdiff = GetTime();

	uint8_t mess[2 * sizeof(int)] = {0};
	uint8_t *messbyte = &mess[0];
	int *messint = reinterpret_cast<int *>(messbyte);

	while (1)
	{
		camera_fb_t *pic = esp_camera_fb_get();

		jpg2bin(pic->buf, pic->len, reinterpret_cast<uint8_t *>(&img.first()), PIC_ENUM); // JPG_SCALE_2X JPG_SCALE_NONE

		esp_camera_fb_return(pic);

		// map = img > int8_t(64);
		map = sobel_operator(img);

		std::cout << map;

		// for (int i = 0; i < 120; ++i)
		// 	map.dilate();

		ESP_LOGI(TAG, "pic_parse: %lf ms", GetTime() - tdiff);
		tdiff = GetTime();

		messint[0] = 120;
		messint[1] = 36;

		i2c_master_write_to_device(I2C_NUM_0, 0x11, messbyte, sizeof(mess), 0);

		// vTaskDelay(1);
		break;
	}

	free(&img);
	free(&map);

	// ESP_LOGI(TAG, "Diff: %i ms, %lf ms", esp_log_timestamp() - startTick, GetTime() - startTime);
	// startTick = esp_log_timestamp();
	// startTime = GetTime();
	// vTaskDelay(pdMS_TO_TICKS(1000));

	// deinit_sd(card);

	return;
}
