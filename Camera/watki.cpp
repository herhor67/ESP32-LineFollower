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

// Camera
#include <esp_camera.h>
#include <img_converters.h>

// Memory
#include <esp_heap_caps.h>

// SD card
#include <sys/unistd.h>
#include <sys/stat.h>
#include <esp_vfs_fat.h>
#include <sdmmc_cmd.h>
#include <driver/sdmmc_host.h>

// Image
#include "jpg2bin.h"
#include "BitMatrix.h"

// SD card
#define MOUNT_POINT "/sdcard"

// Camera
#define CAM_PIN_PWDN GPIO_NUM_32
#define CAM_PIN_RESET GPIO_NUM_NC
#define CAM_PIN_XCLK GPIO_NUM_0
#define CAM_PIN_SIOD GPIO_NUM_26
#define CAM_PIN_SIOC GPIO_NUM_27

#define CAM_PIN_D7 GPIO_NUM_35
#define CAM_PIN_D6 GPIO_NUM_34
#define CAM_PIN_D5 GPIO_NUM_39
#define CAM_PIN_D4 GPIO_NUM_36
#define CAM_PIN_D3 GPIO_NUM_21
#define CAM_PIN_D2 GPIO_NUM_19
#define CAM_PIN_D1 GPIO_NUM_18
#define CAM_PIN_D0 GPIO_NUM_5
#define CAM_PIN_VSYNC GPIO_NUM_25
#define CAM_PIN_HREF GPIO_NUM_23
#define CAM_PIN_PCLK GPIO_NUM_22

#define PIC_H 120
#define PIC_W 160

static const char *TAG = "[" __TIME__
						 "]LnDtctr"; // 🅱

#include <esp_timer.h>
double GetTime()
{
	return (double)esp_timer_get_time() / 1000;
}
static inline int64_t get_time_us()
{
	return esp_timer_get_time();
}

static esp_err_t init_camera()
{
	camera_config_t camera_config = {
		.pin_pwdn = CAM_PIN_PWDN,
		.pin_reset = CAM_PIN_RESET,
		.pin_xclk = CAM_PIN_XCLK,
		.pin_sscb_sda = CAM_PIN_SIOD,
		.pin_sscb_scl = CAM_PIN_SIOC,

		.pin_d7 = CAM_PIN_D7,
		.pin_d6 = CAM_PIN_D6,
		.pin_d5 = CAM_PIN_D5,
		.pin_d4 = CAM_PIN_D4,
		.pin_d3 = CAM_PIN_D3,
		.pin_d2 = CAM_PIN_D2,
		.pin_d1 = CAM_PIN_D1,
		.pin_d0 = CAM_PIN_D0,
		.pin_vsync = CAM_PIN_VSYNC,
		.pin_href = CAM_PIN_HREF,
		.pin_pclk = CAM_PIN_PCLK,

		// XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
		.xclk_freq_hz = 40 * 1000 * 1000,
		.ledc_timer = LEDC_TIMER_0,
		.ledc_channel = LEDC_CHANNEL_0,

		.pixel_format = PIXFORMAT_JPEG, // YUV422,GRAYSCALE,RGB565,JPEG
		.frame_size = FRAMESIZE_QQVGA,	// QQVGA-UXGA Do not use sizes above QVGA when not JPEG FRAMESIZE_QVGA FRAMESIZE_QQVGA

		.jpeg_quality = 10,				  // 0-63 lower number means higher quality
		.fb_count = 2,					  // if more than one, i2s runs in continuous mode. Use only with JPEG
		.fb_location = CAMERA_FB_IN_DRAM, // CAMERA_FB_IN_DRAM CAMERA_FB_IN_PSRAM
		.grab_mode = CAMERA_GRAB_LATEST,  // CAMERA_GRAB_LATEST
	};

	esp_err_t err = esp_camera_init(&camera_config);

	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "Camera Init Failed");
		return err;
	}

	sensor_t *s = esp_camera_sensor_get();
	s->set_brightness(s, 0);			   // -2 to 2
	s->set_contrast(s, 2);				   // -2 to 2
	s->set_saturation(s, 2);			   // -2 to 2
	s->set_special_effect(s, 0);		   // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
	s->set_whitebal(s, 1);				   // 0 = disable , 1 = enable
	s->set_awb_gain(s, 1);				   // 0 = disable , 1 = enable
	s->set_wb_mode(s, 0);				   // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
	s->set_exposure_ctrl(s, 1);			   // 0 = disable , 1 = enable
	s->set_aec2(s, 0);					   // 0 = disable , 1 = enable
	s->set_ae_level(s, 0);				   // -2 to 2
	s->set_aec_value(s, 300);			   // 0 to 1200
	s->set_gain_ctrl(s, 1);				   // 0 = disable , 1 = enable
	s->set_agc_gain(s, 0);				   // 0 to 30
	s->set_gainceiling(s, GAINCEILING_2X); // 0 to 6
	s->set_bpc(s, 0);					   // 0 = disable , 1 = enable
	s->set_wpc(s, 1);					   // 0 = disable , 1 = enable
	s->set_raw_gma(s, 1);				   // 0 = disable , 1 = enable
	s->set_lenc(s, 1);					   // 0 = disable , 1 = enable
	s->set_hmirror(s, 0);				   // 0 = disable , 1 = enable
	s->set_vflip(s, 0);					   // 0 = disable , 1 = enable
	s->set_dcw(s, 1);					   // 0 = disable , 1 = enable
	s->set_colorbar(s, 0);				   // 0 = disable , 1 = enable

	return ESP_OK;
}

static sdmmc_card_t init_sd()
{
	const char mount_point[] = MOUNT_POINT;

	sdmmc_host_t host_cfg = SDMMC_HOST_DEFAULT();

	sdmmc_slot_config_t slot_cfg = {
#ifdef SOC_SDMMC_USE_GPIO_MATRIX
		.clk = GPIO_NUM_14,
		.cmd = GPIO_NUM_15,
		.d0 = GPIO_NUM_2,
		.d1 = GPIO_NUM_4,
		.d2 = GPIO_NUM_12,
		.d3 = GPIO_NUM_13,
		.d4 = GPIO_NUM_NC,
		.d5 = GPIO_NUM_NC,
		.d6 = GPIO_NUM_NC,
		.d7 = GPIO_NUM_NC,
#endif
		.cd = SDMMC_SLOT_NO_CD,
		.wp = SDMMC_SLOT_NO_WP,
		.width = 1, // 1- or 4-wire modes
		.flags = 0 | SDMMC_SLOT_FLAG_INTERNAL_PULLUP,
	};

	esp_vfs_fat_sdmmc_mount_config_t mount_cfg = {
		.format_if_mount_failed = false,
		.max_files = 5,
		.allocation_unit_size = 16 * 512,
	};

	sdmmc_card_t *card;

	ESP_LOGI(TAG, "Initializing SD card, using SDMMC peripheral. Mounting filesystem...");
	esp_err_t ret = esp_vfs_fat_sdmmc_mount(mount_point, &host_cfg, &slot_cfg, &mount_cfg, &card);

	if (ret != ESP_OK)
	{
		if (ret == ESP_FAIL)
			ESP_LOGE(TAG, "Failed to mount filesystem.");
		else
			ESP_LOGE(TAG, "Failed to initialize the card (%s). Make sure SD card is present and lines have pull-up resistors in place.", esp_err_to_name(ret));

		return sdmmc_card_t();
	}
	ESP_LOGI(TAG, "Filesystem mounted");

	// Card has been initialized, print its properties
	sdmmc_card_print_info(stdout, card);

	return *card;
}

static void deinit_sd(sdmmc_card_t &card)
{
	esp_vfs_fat_sdcard_unmount(MOUNT_POINT, &card);
	ESP_LOGI(TAG, "Card unmounted");
}

// void pic_take(void *);
// void pic_convert(void *);
// void pic_parse(void *);

// void pic_take(void *pvParameter)
// {
// 	ESP_LOGI(TAG, "pic_take init");
// 	while (1)
// 	{
// 		ESP_LOGI(TAG, "pic_take loop");
// 		camera_fb_t *pic = esp_camera_fb_get();

// 		xTaskCreate(&pic_convert, "pic_convert", configMINIMAL_STACK_SIZE, pic, 5, nullptr);
// 	}
// }

// void pic_convert(void *arg)
// {
// 	ESP_LOGI(TAG, "pic_convert");
// 	camera_fb_t *pic = static_cast<camera_fb_t *>(arg);

// 	// Matrix<uint8_t, PIC_H, PIC_W>
// 	auto *img = (Matrix<uint8_t, PIC_H, PIC_W> *)heap_caps_calloc(1, sizeof(Matrix<uint8_t, PIC_H, PIC_W>), MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);

// 	jpg2bin(pic->buf, pic->len, &img->first(), JPG_SCALE_NONE); // JPG_SCALE_2X JPG_SCALE_NONE

// 	esp_camera_fb_return(pic);

// 	xTaskCreate(&pic_parse, "pic_parse", configMINIMAL_STACK_SIZE, img, 5, nullptr);

// 	vTaskDelete(NULL);
// }

// void pic_parse(void *img)
// {
// 	ESP_LOGI(TAG, "pic_parse");
// 	vTaskDelete(NULL);
// }

extern "C" void app_main(void)
{
	ESP_LOGI(TAG, "Pre-init xD wwwwwwww ttttttttttt fffffffff xxxxxxxxx ddddddddddddd");

	ESP_LOGI(TAG, "Post-init wwwwwwwwwwwwwww tf");

	// 0-5, 12-15, but lessgo everything
	for (int p = 0; p < SOC_GPIO_PIN_COUNT; ++p)
	{
		PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[p], PIN_FUNC_GPIO);
		gpio_reset_pin(static_cast<gpio_num_t>(p));
	}

	gpio_set_direction(GPIO_NUM_4, GPIO_MODE_OUTPUT);
	gpio_set_direction(GPIO_NUM_33, GPIO_MODE_OUTPUT);

	esp_err_t cam = init_camera();
	if (cam != ESP_OK)
		return;

	// sdmmc_card_t card = init_sd();
	// if (!card.is_mem)
	// 	return;

	// gpio_config(&gpio4_conf);

	ESP_LOGI(TAG, "Pre-task");

	//	TaskHandle_t pic_take_handle = NULL; // nvs_flash_init();

	//	xTaskCreate(&pic_take, "pic_take", configMINIMAL_STACK_SIZE, NULL, 5, &pic_take_handle);

	ESP_LOGI(TAG, "Post-task");

	double startTime;
	uint32_t startTick;

	//	BitMatrix<120, 160> &imgbw = *(BitMatrix<120, 160> *)heap_caps_calloc(1, sizeof(BitMatrix<120, 160>), MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL);

	gpio_set_direction(GPIO_NUM_4, GPIO_MODE_OUTPUT);

	while (1)
	{

		ESP_LOGI(TAG, "While");
		// ESP_LOGI(TAG, "Taking picture...");

		startTick = esp_log_timestamp();
		startTime = GetTime();

		// gpio_set_level(GPIO_NUM_4, 1);
		// gpio_set_level(GPIO_NUM_4, 0);

		// ESP_LOGI(TAG, "Diff: %i ms, %lf ms", esp_log_timestamp() - startTick, GetTime() - startTime);

		ESP_LOGI(TAG, "Diff: %i ms, %lf ms", esp_log_timestamp() - startTick, GetTime() - startTime);

		vTaskDelay(pdMS_TO_TICKS(1000));
	}
	// free(RGBbuf);

	// deinit_sd(card);

	return;
}
