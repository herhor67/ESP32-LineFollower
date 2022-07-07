// #define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE
#define LOG_LOCAL_LEVEL ESP_LOG_WARN

// CPP headers
#include <algorithm>
#include <functional>
#include <string>
#include <vector>

// ESP system
#include <esp_log.h>
#include <esp_system.h>
//#include <nvs_flash.h>
#include <sys/param.h>
#include <string.h>

// FreeRTOS
// #include <freertos/FreeRTOS.h>
// #include <freertos/task.h>

// Memory
#include <esp_heap_caps.h>

// Image
#include "jpg2bin.h"
#include "BitMatrix.h"

// Other
#include "functions.h"
#include "BitsetOps.h"

//
#define PIN_FLASH GPIO_NUM_4
#define PIN_BLINK GPIO_NUM_33
#define PIN_STATUS GPIO_NUM_12
#define PIN_EN_IN GPIO_NUM_2

const char *TAG = "[" __TIME__
				  "]LnDtctr🅱 ";

constexpr auto vw_mdl = model_generator(70, 15, 66);
constexpr auto px_crds = pixel_generate_pos();
constexpr auto px_prms = pixel_generate_params(px_crds, vw_mdl);

extern "C" void app_main(void)
{
	// 0-5, 12-15, but lessgo everything
	for (int p : {12, 13, 14, 15})
	{
		PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[p], PIN_FUNC_GPIO);
		gpio_reset_pin(static_cast<gpio_num_t>(p));
	}

	gpio_set_direction(PIN_FLASH, GPIO_MODE_OUTPUT);  // flash LED
	gpio_set_direction(PIN_BLINK, GPIO_MODE_OUTPUT);  // blink LED
	gpio_set_direction(PIN_STATUS, GPIO_MODE_OUTPUT); // status pin

	gpio_set_direction(PIN_EN_IN, GPIO_MODE_INPUT); // enable signal
	gpio_set_pull_mode(PIN_EN_IN, GPIO_PULLDOWN_ONLY);

	gpio_set_level(PIN_FLASH, 0); // disable flash LED

	esp_err_t cam = init_camera();
	if (cam != ESP_OK)
		return;

	// sdmmc_card_t card = init_sd();
	// if (!card.is_mem)
	// 	return;

	esp_err_t i2c = init_i2c();
	if (i2c != ESP_OK)
		return;

	uint32_t startTick = esp_log_timestamp();
	double startTime = GetTime();

	// PREPARE LOOP VARIABLES/STORAGE
	rgb_img_t &img = *(rgb_img_t *)heap_caps_calloc(1, sizeof(rgb_img_t), MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM); // MALLOC_CAP_INTERNAL MALLOC_CAP_SPIRAM
	uint8_t *imgptr = reinterpret_cast<uint8_t *>(&img.first());

	rgb_img_t &sob = *(rgb_img_t *)heap_caps_calloc(1, sizeof(rgb_img_t), MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);

	bin_img_t &edg = *(bin_img_t *)heap_caps_calloc(1, sizeof(bin_img_t), MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);

	bin_img_t &map = *(bin_img_t *)heap_caps_calloc(1, sizeof(bin_img_t), MALLOC_CAP_8BIT | MALLOC_CAP_SPIRAM);

	std::bitset<Brdrln> border;

	std::vector<px_params_t> path_list;
	path_list.reserve(10);
	px_params_t current_path;

	float mess[2] = {0.0f, 0.0f};
	uint8_t *messbyte = reinterpret_cast<uint8_t *>(&mess);

	bool blinker = false;

	// int i = 0;
	// while (1)
	// {
	// 	++i;
	// 	mess[0] = i % 47;
	// 	mess[1] = i % 69;
	// 	i2c_master_write_to_device(I2C_NUM_0, 0x67, messbyte, sizeof(mess), 0);
	// 	ESP_LOGI(TAG, "Diff: %i ms, %lf ms, Z: %f, X: %f", esp_log_timestamp() - startTick, GetTime() - startTime, mess[0], mess[1]);
	// 	startTick = esp_log_timestamp();
	// 	startTime = GetTime();
	// 	vTaskDelay(30);
	// }
	// return;

	gpio_set_level(PIN_STATUS, 1); // enable status pin
	while (!gpio_get_level(PIN_EN_IN))
		vTaskDelay(1);

	while (1)
	{
		gpio_set_level(PIN_BLINK, blinker = !blinker);

		mess[0] = mess[1] = 0.0f;
		//==============================//
		//              ||              //
		//==============================//
		//==============================//
		//           TAKE PIC           //
		//==============================//
		camera_fb_t *pic = esp_camera_fb_get();

		// FILE *fp = fopen(MOUNT_POINT "/photo.jpg", "w");
		// if (fp != NULL)
		// {
		// 	fwrite(pic->buf, sizeof(uint8_t), pic->len, fp);
		// 	fclose(fp);
		// }

		//==============================//
		//         CONVERT PIC          //
		//==============================//
		jpg2bin(pic->buf, pic->len, imgptr, PIC_ENUM); // JPG_SCALE_2X JPG_SCALE_NONE
		esp_camera_fb_return(pic);

		// fp = fopen(MOUNT_POINT "/photo.raw", "w");
		// if (fp != NULL)
		// {
		// 	fwrite(imgptr, sizeof(uint8_t), Zmax * Xmax, fp);
		// 	fclose(fp);
		// }

		//		std::cout << img;

		//==============================//
		//         FIND CENTER          //
		//==============================//
		size_t ctrZ = vw_mdl.Zctr;
		size_t ctrX = 1;
		int8_t minEl = img(ctrX, ctrZ);
		int8_t maxEl = img(ctrX, ctrZ);
		for (size_t x = 1; x < Xmax - 1; ++x)
		{
			int8_t el = img(x, ctrZ);
			if (el < minEl)
			{
				minEl = el;
				ctrX = x;
			}
			if (el > maxEl)
				maxEl = el;
		}
		size_t minX = ctrX;
		size_t maxX = ctrX;
		int8_t maxdiff = std::max(5, (maxEl - minEl) / 10); // std::max(5, minEl / 2);
		for (size_t x = ctrX; x < Xmax - 1; ++x)
		{
			if (img(x, ctrZ) > minEl + maxdiff)
				break;
			maxX = x;
		}
		for (size_t x = ctrX; x > 0; --x)
		{
			if (img(x, ctrZ) > minEl + maxdiff)
				break;
			minX = x;
		}
		ctrX = (minX + maxX) / 2;

		// std::cout << "ctrZ: " << ctrZ << "\t Xm: " << minX << "\t XM: " << maxX << "\t Xc: " << ctrX << std::endl;

		//==============================//
		//          FILL PATH           //
		//==============================//

		sobel_operator(img, sob);

		// fp = fopen(MOUNT_POINT "/sobel.raw", "w");
		// if (fp != NULL)
		// {
		// 	fwrite(&sob(0, 0), sizeof(uint8_t), Zmax * Xmax, fp);
		// 	fclose(fp);
		// }

		imbinarize(sob, edg);

		// std::cout << edg;

		edg.Fpq();

		//==============================//
		//           TAKE PIC           //
		//==============================//
		map.Opq();
		map(ctrX, ctrZ) = 1;

		fill_row(map, edg, ctrX);
		fill_rows_cmx(map, edg, ctrX);
		fill_rows_cmn(map, edg, ctrX);
		fill_rows_mxc(map, edg, ctrX);
		fill_rows_mnc(map, edg, ctrX);
		fill_rows_cmx(map, edg, ctrX);
		fill_rows_cmn(map, edg, ctrX);

		// std::cout << map;

		//==============================//
		//          GET BORDER          //
		//==============================//
		for (size_t i = 0; i < Brdrln; ++i)
			border[i] = map.test(Xmax - px_crds[i].second - 1, px_crds[i].first);

		border |= rotr1(border) | rotl1(border);

		// std::cout << border.to_string('.', '#') << std::endl;

		//==============================//
		//         GROUP PATHS          //
		//==============================//
		path_list.clear();
		current_path.clear();

		for (size_t i = 0; i < Brdrln; ++i)
		{
			if (border.test(i))
				current_path += px_prms[i];
			else if (current_path.cnt != 0)
			{
				path_list.push_back(current_path);
				current_path.clear();
			}
		}

		if (current_path.cnt != 0)
		{
			if (border.test(0) && path_list.size())
				path_list[0] += current_path;
			else
				path_list.push_back(current_path);
		}

		//==============================//
		//       NORM, FIND BEST        //
		//==============================//

		std::for_each(path_list.begin(), path_list.end(), std::mem_fn(&px_params_t::norm));

		// std::for_each(path_list.begin(), path_list.end(), [](const px_params_t &r)
		// 			  { std::cout << r.dst << '\t' << r.sin << '\t' << r.cos << '\t' << +r.cnt << std::endl; });

		if (path_list.empty())
			path_list.push_back({0, 0, 0, 1});

		auto best = std::max_element(path_list.begin(), path_list.end(), [](const px_params_t &a, const px_params_t &b)
									 { return a.sin < b.sin; });

		// std::cout << "Index: " << std::distance(path_list.begin(), best) << std::endl;

		//==============================//
		//           SEND I2C           //
		//==============================//

		mess[0] = best->sin * best->dst;
		mess[1] = best->cos * best->dst;

		ESP_LOGI(TAG, "TARGET: Z=%f X=%f", mess[0], mess[1]);

		if (gpio_get_level(PIN_EN_IN))
			i2c_master_write_to_device(I2C_NUM_0, 0x67, messbyte, sizeof(mess), 0);

		ESP_LOGI(TAG, "Diff: %i ms, %lf ms", esp_log_timestamp() - startTick, GetTime() - startTime);
		startTick = esp_log_timestamp();
		startTime = GetTime();
		vTaskDelay(1);

		// break;
	}

	free(&img);
	free(&map);

	ESP_LOGI(TAG, "Diff: %i ms, %lf ms", esp_log_timestamp() - startTick, GetTime() - startTime);
	startTick = esp_log_timestamp();
	startTime = GetTime();
	vTaskDelay(pdMS_TO_TICKS(1000));

	// deinit_sd(card);

	return;
}