

#include <string>

#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_camera.h"

#include <sys/unistd.h>
#include <sys/stat.h>
#include "esp_vfs_fat.h"
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"

#define MOUNT_POINT "/sdcard"

#define CAM_PIN_PWDN 32
#define CAM_PIN_RESET -1
#define CAM_PIN_XCLK 0
#define CAM_PIN_SIOD 26
#define CAM_PIN_SIOC 27

#define CAM_PIN_D7 35
#define CAM_PIN_D6 34
#define CAM_PIN_D5 39
#define CAM_PIN_D4 36
#define CAM_PIN_D3 21
#define CAM_PIN_D2 19
#define CAM_PIN_D1 18
#define CAM_PIN_D0 5
#define CAM_PIN_VSYNC 25
#define CAM_PIN_HREF 23
#define CAM_PIN_PCLK 22

static const char *TAG = "Photographer";

static camera_config_t camera_config = {
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
	.xclk_freq_hz = 20 * 1000 * 1000,
	.ledc_timer = LEDC_TIMER_0,
	.ledc_channel = LEDC_CHANNEL_0,

	.pixel_format = PIXFORMAT_JPEG, // YUV422,GRAYSCALE,RGB565,JPEG
	.frame_size = FRAMESIZE_QVGA,	// QQVGA-UXGA Do not use sizes above QVGA when not JPEG

	.jpeg_quality = 12, // 0-63 lower number means higher quality
	.fb_count = 1,		// if more than one, i2s runs in continuous mode. Use only with JPEG
	.fb_location = CAMERA_FB_IN_PSRAM,
	.grab_mode = CAMERA_GRAB_WHEN_EMPTY,
};

static esp_err_t init_camera()
{
	// initialize the camera
	esp_err_t err = esp_camera_init(&camera_config);
	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "Camera Init Failed");
		return err;
	}

	return ESP_OK;
}

static sdmmc_card_t init_sd()
{
	esp_vfs_fat_sdmmc_mount_config_t mount_config =
		{
			.format_if_mount_failed = false,
			.max_files = 5,
			.allocation_unit_size = 16 * 1024,
		};

	sdmmc_card_t *card;
	const char mount_point[] = MOUNT_POINT;
	ESP_LOGI(TAG, "Initializing SD card");

	ESP_LOGI(TAG, "Using SDMMC peripheral");
	sdmmc_host_t host = SDMMC_HOST_DEFAULT();

	// Modify slot_config.gpio_cd and slot_config.gpio_wp if your board has these signals.
	sdmmc_slot_config_t slot_config =
		{
			.cd = SDMMC_SLOT_NO_CD,
			.wp = SDMMC_SLOT_NO_WP,
			.width = 1, // 1- or 4-wire modes
			.flags = 0,
		};

	// On chips where the GPIOs used for SD card can be configured, set them in the slot_config structure:
#ifdef SOC_SDMMC_USE_GPIO_MATRIX
	slot_config.clk = GPIO_NUM_14;
	slot_config.cmd = GPIO_NUM_15;
	slot_config.d0 = GPIO_NUM_2;
	slot_config.d1 = GPIO_NUM_4;
	slot_config.d2 = GPIO_NUM_12;
	slot_config.d3 = GPIO_NUM_13;
#endif

	// Enable internal pullups on enabled pins. The internal pullups
	// are insufficient however, please make sure 10k external pullups are
	// connected on the bus. This is for debug / example purpose only.
	slot_config.flags |= SDMMC_SLOT_FLAG_INTERNAL_PULLUP;

	ESP_LOGI(TAG, "Mounting filesystem");
	esp_err_t ret = esp_vfs_fat_sdmmc_mount(mount_point, &host, &slot_config, &mount_config, &card);

	if (ret != ESP_OK)
	{
		if (ret == ESP_FAIL)
		{
			ESP_LOGE(TAG, "Failed to mount filesystem. "
						  "If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
		}
		else
		{
			ESP_LOGE(TAG, "Failed to initialize the card (%s). "
						  "Make sure SD card lines have pull-up resistors in place.",
						  esp_err_to_name(ret));
		}
		return sdmmc_card_t();
	}
	ESP_LOGI(TAG, "Filesystem mounted");

	// Card has been initialized, print its properties
	sdmmc_card_print_info(stdout, card);

	return *card;
}


extern "C" void app_main(void)
{
	esp_err_t cam = init_camera();
	if (cam != ESP_OK)
		return;
	
	sdmmc_card_t card = init_sd();
	if (!card.is_mem)
		return;

	size_t picnum = 0;

	while (1)
	{
		picnum++;

		ESP_LOGI(TAG, "Taking picture...");
		camera_fb_t *pic = esp_camera_fb_get();

		// use pic->buf to access the image
		ESP_LOGI(TAG, "Picture taken! Its size was: %zuB, HxW: %zux%zu", pic->len, pic->height, pic->width);


		// std::string fname = (std::string)MOUNT_POINT + "/file" + std::to_string(picnum) + ".jpg";

		// ESP_LOGI(TAG, "Opening file %s", fname.c_str());
		// FILE *fp = fopen(fname.c_str(), "w");

		// if (fp == NULL)
		// {
		// 	ESP_LOGE(TAG, "Failed to open file for writing");
		// 	break;
		// }

		// fwrite(pic->buf, 1, pic->len, fp);
		// ESP_LOGI(TAG, "File written");
		// fclose(fp);
		
		esp_camera_fb_return(pic);

//		vTaskDelay(pdMS_TO_TICKS(5000));
	}

	esp_vfs_fat_sdcard_unmount(MOUNT_POINT, &card);
	ESP_LOGI(TAG, "Card unmounted");

	return;
}
