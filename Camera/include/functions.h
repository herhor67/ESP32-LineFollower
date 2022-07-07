#pragma once

extern const char *TAG;

// Includes
#include <cmath>
#include <tuple>
#include <utility>

// System
#include <esp_log.h>
#include <esp_system.h>
// #include <sys/param.h>

// I2C
#include <driver/i2c.h>

// Camera
#include <esp_camera.h>
#include <img_converters.h>
#include "BitMatrix.h"

// Camera
#define PIC_H 120
#define PIC_W 160
#define PIC_SCALE 2
#define PIC_ENUM JPG_SCALE_2X

using img_crd_t = MinUInt<std::max(PIC_H / PIC_SCALE, PIC_W / PIC_SCALE)>;

constexpr img_crd_t Zmax = PIC_W / PIC_SCALE;
constexpr img_crd_t Xmax = PIC_H / PIC_SCALE;
constexpr size_t Brdrln = size_t(Zmax) * 2 + size_t(Xmax) * 2 - 4;

using rgb_img_t = Matrix<int8_t, Xmax, Zmax>;
using bin_img_t = BitMatrix<Xmax, Zmax>;

// SD card
#include <sys/unistd.h>
#include <sys/stat.h>
#include <esp_vfs_fat.h>
#include <sdmmc_cmd.h>
#include <driver/sdmmc_host.h>

// SD card
#define MOUNT_POINT "/sdcard"

#include <esp_timer.h>

#include "gcem.hpp"

//==============================//
//             MISC             //
//==============================//

template <typename T>
constexpr T deg2rad(T degrees)
{
	constexpr T radperdeg = static_cast<T>(3.14159265358979323846 / 180.0);
	return degrees * radperdeg;
}

double GetTime()
{
	return (double)esp_timer_get_time() / 1000;
}

static inline int64_t get_time_us()
{
	return esp_timer_get_time();
}

//==============================//
//             I2C              //
//==============================//

static esp_err_t init_i2c()
{
	esp_err_t ret;

	i2c_config_t i2c_config = {
		.mode = I2C_MODE_MASTER,
		.sda_io_num = GPIO_NUM_14,
		.scl_io_num = GPIO_NUM_13,
		.sda_pullup_en = GPIO_PULLUP_ENABLE,
		.scl_pullup_en = GPIO_PULLUP_ENABLE,
		.master = {.clk_speed = 500000},
		.clk_flags = I2C_SCLK_SRC_FLAG_FOR_NOMAL, // Optional, you can use I2C_SCLK_SRC_FLAG_* flags to choose i2c source clock here.
	};

	ret = i2c_param_config(I2C_NUM_0, &i2c_config);
	if (ret != ESP_OK)
		return ret;

	ret = i2c_driver_install(I2C_NUM_0, I2C_MODE_MASTER, 0, 0, ESP_INTR_FLAG_IRAM); //
	if (ret != ESP_OK)
		return ret;

	return ESP_OK;
}

//==============================//
//            CAMERA            //
//==============================//

static esp_err_t init_camera()
{
	camera_config_t camera_config = {
		.pin_pwdn = GPIO_NUM_32,
		.pin_reset = GPIO_NUM_NC,
		.pin_xclk = GPIO_NUM_0,
		.pin_sscb_sda = GPIO_NUM_26,
		.pin_sscb_scl = GPIO_NUM_27,

		.pin_d7 = GPIO_NUM_35,
		.pin_d6 = GPIO_NUM_34,
		.pin_d5 = GPIO_NUM_39,
		.pin_d4 = GPIO_NUM_36,
		.pin_d3 = GPIO_NUM_21,
		.pin_d2 = GPIO_NUM_19,
		.pin_d1 = GPIO_NUM_18,
		.pin_d0 = GPIO_NUM_5,
		.pin_vsync = GPIO_NUM_25,
		.pin_href = GPIO_NUM_23,
		.pin_pclk = GPIO_NUM_22,

		// XCLK 20MHz or 10MHz for OV2640 double FPS (Experimental)
		.xclk_freq_hz = 40 * 1000 * 1000,
		.ledc_timer = LEDC_TIMER_0,
		.ledc_channel = LEDC_CHANNEL_0,

		.pixel_format = PIXFORMAT_JPEG, // YUV422,GRAYSCALE,RGB565,JPEG
		.frame_size = FRAMESIZE_QQVGA,	// QQVGA-UXGA Do not use sizes above QVGA when not JPEG FRAMESIZE_QVGA FRAMESIZE_QQVGA

		.jpeg_quality = 10,					 // 0-63 lower number means higher quality
		.fb_count = 2,						 // if more than one, i2s runs in continuous mode. Use only with JPEG
		.fb_location = CAMERA_FB_IN_DRAM,	 // CAMERA_FB_IN_DRAM CAMERA_FB_IN_PSRAM
		.grab_mode = CAMERA_GRAB_WHEN_EMPTY, // CAMERA_GRAB_LATEST CAMERA_GRAB_WHEN_EMPTY
	};

	esp_err_t err = esp_camera_init(&camera_config);

	if (err != ESP_OK)
	{
		ESP_LOGE(TAG, "Camera Init Failed");
		return err;
	}

	sensor_t *s = esp_camera_sensor_get();

	s->set_brightness(s, 0); // int -2 to 2
	s->set_contrast(s, 1);	 // int -2 to 2
	s->set_saturation(s, 1); // int -2 to 2
	s->set_sharpness(s, 1);	 // int -2 to 2
	// s->set_denoise(s, 0);	 // uint ?

	s->set_whitebal(s, 0); // auto white balance 0/1
	s->set_awb_gain(s, 0); // auto white balance gain 0/1 - set to 1 to fix colors
	s->set_wb_mode(s, 0);  // if awb_gain enabled enum(0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)

	s->set_exposure_ctrl(s, 1); // automatic exposure control 0/1
	s->set_aec_value(s, 300);	// 0 to 1200 if exposure_ctrl=0
	s->set_aec2(s, 1);			// automatic exposure control 2 0/1
	s->set_ae_level(s, 0);		// automatic exposure level int -2 to 2

	s->set_gain_ctrl(s, 1);					// 0/1
	s->set_agc_gain(s, 16);					// 0 to 30 ??????????
	s->set_gainceiling(s, GAINCEILING_32X); // 0 to 6

	s->set_bpc(s, 0);	  // black pixel correction 0/1
	s->set_wpc(s, 0);	  // white pixel correction 0/1
	s->set_raw_gma(s, 1); // gamma correction 0/1
	s->set_lenc(s, 0);	  // lens correction 0/1

	s->set_dcw(s, 1);	   // downsizing 0/1
	s->set_colorbar(s, 0); // colorbar 0/1

	s->set_hmirror(s, 0);		 // horizontal mirror 0/1
	s->set_vflip(s, 0);			 // vertical flip 0/1
	s->set_special_effect(s, 0); // enum(0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)

	return ESP_OK;
}

//==============================//
//           SD CARD            //
//==============================//

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

//==============================//
//            MODEL             //
//==============================//

struct view_model_t
{
	img_crd_t Zmax;
	img_crd_t Xmax;
	float cam_height;
	float cam_pitch_tan;
	float lens_twrd_tan;
	float lens_sdws_tan;
	float dz_fwd;
	float dz_bwd;
	float dz_ttl;
	float dx_frnt;
	float dx_back;
	img_crd_t Zctr;
	img_crd_t Xctr;
};

constexpr view_model_t model_generator(float c_h, float c_p, float l_a)
{
	view_model_t t = {};

	t.Zmax = Zmax;
	t.Xmax = Xmax;

	t.cam_height = c_h;
	float cam_pitch = deg2rad(c_p);
	float lens_twrd = deg2rad(l_a * 0.5f);

	t.cam_pitch_tan = gcem::tan(cam_pitch);
	t.lens_twrd_tan = gcem::tan(lens_twrd);
	t.lens_sdws_tan = t.lens_twrd_tan * t.Xmax / t.Zmax;

	t.dz_fwd = t.cam_height * gcem::tan(lens_twrd + cam_pitch);
	t.dz_bwd = t.cam_height * gcem::tan(lens_twrd - cam_pitch);
	t.dz_ttl = t.dz_fwd + t.dz_bwd;

	float ray_fwd = gcem::sqrt(t.cam_height * t.cam_height + t.dz_fwd * t.dz_fwd);
	float ray_bwd = gcem::sqrt(t.cam_height * t.cam_height + t.dz_bwd * t.dz_bwd);

	// float img_fwd = ray_fwd * gcem::sin(lens_twrd);
	// float img_bwd = ray_bwd * gcem::sin(lens_twrd);

	float dst_fwd = ray_fwd * gcem::cos(lens_twrd);
	float dst_bwd = ray_bwd * gcem::cos(lens_twrd);

	t.dx_frnt = dst_fwd * t.lens_sdws_tan;
	t.dx_back = dst_bwd * t.lens_sdws_tan;

	t.Zctr = gcem::floor(Zmax * 0.5f * (1 + t.cam_pitch_tan / t.lens_twrd_tan));
	t.Xctr = gcem::floor(Xmax * 0.5f);

	return t;
}

constexpr std::pair<float, float> px2dst(const std::pair<img_crd_t, img_crd_t> &px_crds, const view_model_t &vm)
{
	float z_px = px_crds.first + 0.5f;
	float x_px = px_crds.second + 0.5f;

	float Zrt_abs = 1.0f - z_px / vm.Zmax;
	float Xrt_abs = x_px / vm.Xmax;

	float Zrt_cnt = 2.0f * Zrt_abs - 1.0f;
	float Xrt_cnt = 2.0f * Xrt_abs - 1.0f;

	float z_dst = vm.cam_height * (vm.cam_pitch_tan + Zrt_cnt * vm.lens_twrd_tan) / (1.0f - vm.cam_pitch_tan * Zrt_cnt * vm.lens_twrd_tan);

	float len_rat = (z_dst + vm.dz_bwd) / vm.dz_ttl;
	float x_wdth = len_rat * vm.dx_frnt + (1.0f - len_rat) * vm.dx_back;

	float x_dst = Xrt_cnt * x_wdth;

	return std::make_pair(z_dst, x_dst);
}

constexpr auto pixel_generate_pos()
{
	std::array<std::pair<img_crd_t, img_crd_t>, Brdrln> border = {};

	img_crd_t Xrds = Xmax / 2;
	size_t idx = 0;

	for (img_crd_t x = Xrds - 1; x >= 1; --x)
	{
		border[idx].first = Zmax - 1;
		border[idx].second = x;
		++idx;
	}

	border[idx].first = Zmax - 1;
	border[idx].second = 0;
	++idx;

	for (img_crd_t z = Zmax - 2; z >= 1; --z)
	{
		border[idx].first = z;
		border[idx].second = 0;
		++idx;
	}

	border[idx].first = 0;
	border[idx].second = 0;
	++idx;

	for (img_crd_t x = 1; x <= Xmax - 2; ++x)
	{
		border[idx].first = 0;
		border[idx].second = x;
		++idx;
	}

	border[idx].first = 0;
	border[idx].second = Xmax - 1;
	++idx;

	for (img_crd_t z = 1; z <= Zmax - 2; ++z)
	{
		border[idx].first = z;
		border[idx].second = Xmax - 1;
		++idx;
	}

	border[idx].first = Zmax - 1;
	border[idx].second = Xmax - 1;
	++idx;

	for (img_crd_t x = Xmax - 2; x >= Xrds; --x)
	{
		border[idx].first = Zmax - 1;
		border[idx].second = x;
		++idx;
	}

	return border;
}

struct px_params_t
{
	float dst = 0;
	float sin = 0;
	float cos = 0;
	img_crd_t cnt = 0;

	px_params_t &operator+=(const px_params_t &rhs)
	{
		dst += rhs.dst;
		sin += rhs.sin;
		cos += rhs.cos;
		cnt += rhs.cnt;
		return *this;
	}
	px_params_t &clear()
	{
		dst = 0;
		sin = 0;
		cos = 0;
		cnt = 0;
		return *this;
	}
	px_params_t &norm()
	{
		dst /= cnt;
		sin /= cnt;
		cos /= cnt;
		cnt = 1;
		return *this;
	}
};

constexpr auto pixel_generate_params(const std::array<std::pair<img_crd_t, img_crd_t>, Brdrln> &px_crds, const view_model_t &vm)
{
	std::array<px_params_t, Brdrln> table = {};

	for (size_t i = 0; i < Brdrln; ++i)
	{
		auto zx = px2dst(px_crds[i], vm);
		float dist = gcem::sqrt(zx.first * zx.first + zx.second * zx.second);
		table[i] = {
			.dst = dist,
			.sin = zx.first / dist,
			.cos = zx.second / dist,
			.cnt = 1,
		};
	}

	return table;
}

//==============================//
//            SOBEL             //
//==============================//

void sobel_operator(const rgb_img_t &img, rgb_img_t &sob)
{
	constexpr int w[] = {1, 2, 1};
	constexpr int div = 1 + 2 + 1;
	// constexpr int[] w = {3, 10, 3};
	// constexpr int div = 3 + 10 + 3;
	// constexpr int[] w = {47, 162, 47};
	// constexpr int div = 47 + 162 + 47;

	auto fun = [&w, &div, &sob](ptrdiff_t i, ptrdiff_t j, int8_t e11, int8_t e12, int8_t e13, int8_t e21, int8_t e22, int8_t e23, int8_t e31, int8_t e32, int8_t e33) -> void
	{
		int Gi = w[0] * (e11 - e13) + w[1] * (e21 - e23) + w[2] * (e31 - e33);
		int Gj = w[0] * (e11 - e31) + w[1] * (e12 - e32) + w[2] * (e13 - e33);

		int val = std::abs(Gi) + std::abs(Gj);
		// int val = std::sqrt(Gi * Gi + Gj * Gj);
		sob(i, j) = std::min(val / div, (int)std::numeric_limits<rgb_img_t::elem_t>::max());
	};

	for (ptrdiff_t ic = 0; ic < Xmax; ++ic)
		for (ptrdiff_t jc = 0; jc < Zmax; ++jc)
		{
			ptrdiff_t im = std::max(ic - 1, 0);
			ptrdiff_t ip = std::min(ic + 1, Xmax - 1);
			ptrdiff_t jm = std::max(jc - 1, 0);
			ptrdiff_t jp = std::min(jc + 1, Zmax - 1);
			fun(ic, jc,
				img(im, jm), img(im, jc), img(im, jp),
				img(ic, jm), img(ic, jc), img(ic, jp),
				img(ip, jm), img(ip, jc), img(ip, jp));
		}
}

/*/

void sobel_operator(const rgb_img_t &img, rgb_img_t &sob)
{
	constexpr int w[] = {1, 2, 1};
	constexpr int div = 1 + 2 + 1;
	// constexpr int[] w = {3, 10, 3};
	// constexpr int div = 3 + 10 + 3;
	// constexpr int[] w = {47, 162, 47};
	// constexpr int div = 47 + 162 + 47;

	auto fun = [&w, &div, &sob](size_t i, size_t j, int8_t e11, int8_t e12, int8_t e13, int8_t e21, int8_t e22, int8_t e23, int8_t e31, int8_t e32, int8_t e33) -> void
	{
		int Gi = w[0] * (e11 - e13) + w[1] * (e21 - e23) + w[2] * (e31 - e33);
		int Gj = w[0] * (e11 - e31) + w[1] * (e12 - e32) + w[2] * (e13 - e33);

		int val = std::abs(Gi) + std::abs(Gj);
		// int val = std::sqrt(Gi * Gi + Gj * Gj);
		sob(i, j) = std::min(val / div, (int)std::numeric_limits<rgb_img_t::elem_t>::max());
	};

	size_t i = 0;
	size_t j = 0;
	fun(i, j, img(i, j), img(i, j), img(i, j + 1), img(i, j), img(i, j), img(i, j + 1), img(i + 1, j), img(i + 1, j), img(i + 1, j + 1));

	for (size_t j = 1; j < Zmax - 1; ++j)
		fun(i, j, img(i, j - 1), img(i, j), img(i, j + 1), img(i, j - 1), img(i, j), img(i, j + 1), img(i + 1, j - 1), img(i + 1, j), img(i + 1, j + 1));

	fun(i, j, img(i, j - 1), img(i, j), img(i, j), img(i, j - 1), img(i, j), img(i, j), img(i + 1, j - 1), img(i + 1, j), img(i + 1, j));

	for (i = 1; i < Xmax - 1; ++i)
	{
		j = 0;
		fun(i, j, img(i - 1, j), img(i - 1, j), img(i - 1, j + 1), img(i, j), img(i, j), img(i, j + 1), img(i + 1, j), img(i + 1, j), img(i + 1, j + 1));

		for (j = 1; j < Zmax - 1; ++j)
			fun(i, j, img(i - 1, j - 1), img(i - 1, j), img(i - 1, j + 1), img(i, j - 1), img(i, j), img(i, j + 1), img(i + 1, j - 1), img(i + 1, j), img(i + 1, j + 1));

		fun(i, j, img(i - 1, j - 1), img(i - 1, j), img(i - 1, j), img(i, j - 1), img(i, j), img(i, j), img(i + 1, j - 1), img(i + 1, j), img(i + 1, j));
	}

	j = 0;
	fun(i, j, img(i - 1, j), img(i - 1, j), img(i - 1, j + 1), img(i, j), img(i, j), img(i, j + 1), img(i, j), img(i, j), img(i, j + 1));

	for (j = 1; j < Zmax - 1; ++j)
		fun(i, j, img(i - 1, j - 1), img(i - 1, j), img(i - 1, j + 1), img(i, j - 1), img(i, j), img(i, j + 1), img(i, j - 1), img(i, j), img(i, j + 1));

	fun(i, j, img(i - 1, j - 1), img(i - 1, j), img(i - 1, j), img(i, j - 1), img(i, j), img(i, j), img(i, j - 1), img(i, j), img(i, j));
}


void sobel_operator(const rgb_img_t &img, bin_img_t &map)
{
	constexpr int w[] = {1, 2, 1};
	constexpr int div = 1 + 2 + 1;
	// constexpr int[] w = {3, 10, 3};
	// constexpr int div = 3 + 10 + 3;
	// constexpr int[] w = {47, 162, 47};
	// constexpr int div = 47 + 162 + 47;

	auto fun = [&w, &div, &map](size_t i, size_t j, int8_t e11, int8_t e12, int8_t e13, int8_t e21, int8_t e22, int8_t e23, int8_t e31, int8_t e32, int8_t e33) -> void
	{
		int Gi = w[0] * (e11 - e13) + w[1] * (e21 - e23) + w[2] * (e31 - e33);
		int Gj = w[0] * (e11 - e31) + w[1] * (e12 - e32) + w[2] * (e13 - e33);

		int val = std::abs(Gi) + std::abs(Gj);
		// int val = std::sqrt(Gi * Gi + Gj * Gj);
		map(i, j) = val > (16 * div);
	};

	size_t i = 0;
	size_t j = 0;
	fun(i, j, img(i, j), img(i, j), img(i, j + 1), img(i, j), img(i, j), img(i, j + 1), img(i + 1, j), img(i + 1, j), img(i + 1, j + 1));

	for (size_t j = 1; j < Zmax - 1; ++j)
		fun(i, j, img(i, j - 1), img(i, j), img(i, j + 1), img(i, j - 1), img(i, j), img(i, j + 1), img(i + 1, j - 1), img(i + 1, j), img(i + 1, j + 1));

	fun(i, j, img(i, j - 1), img(i, j), img(i, j), img(i, j - 1), img(i, j), img(i, j), img(i + 1, j - 1), img(i + 1, j), img(i + 1, j));

	for (i = 1; i < Xmax - 1; ++i)
	{
		j = 0;
		fun(i, j, img(i - 1, j), img(i - 1, j), img(i - 1, j + 1), img(i, j), img(i, j), img(i, j + 1), img(i + 1, j), img(i + 1, j), img(i + 1, j + 1));

		for (j = 1; j < Zmax - 1; ++j)
			fun(i, j, img(i - 1, j - 1), img(i - 1, j), img(i - 1, j + 1), img(i, j - 1), img(i, j), img(i, j + 1), img(i + 1, j - 1), img(i + 1, j), img(i + 1, j + 1));

		fun(i, j, img(i - 1, j - 1), img(i - 1, j), img(i - 1, j), img(i, j - 1), img(i, j), img(i, j), img(i + 1, j - 1), img(i + 1, j), img(i + 1, j));
	}

	j = 0;
	fun(i, j, img(i - 1, j), img(i - 1, j), img(i - 1, j + 1), img(i, j), img(i, j), img(i, j + 1), img(i, j), img(i, j), img(i, j + 1));

	for (j = 1; j < Zmax - 1; ++j)
		fun(i, j, img(i - 1, j - 1), img(i - 1, j), img(i - 1, j + 1), img(i, j - 1), img(i, j), img(i, j + 1), img(i, j - 1), img(i, j), img(i, j + 1));

	fun(i, j, img(i - 1, j - 1), img(i - 1, j), img(i - 1, j), img(i, j - 1), img(i, j), img(i, j), img(i, j - 1), img(i, j), img(i, j));
}
//*/

//==============================//
//          IMBINARIZE          //
//==============================//

void imbinarize(const rgb_img_t &sob, bin_img_t &ibw)
{
	int8_t mxm = sob.max();
	int8_t mnm = sob.min();
	int8_t avg = sob.avg();
	int8_t thr = avg + (mxm - avg) / 20;

	for (size_t i = 0; i < Xmax; ++i)
		for (size_t j = 0; j < Zmax; ++j)
			ibw.set(i, j, sob(i, j) > thr);
}

//==============================//
//         ROW FILLERS          //
//==============================//

void fill_row(bin_img_t &map, bin_img_t &edg, size_t x)
{
	static bin_img_t::row_t old;
	do
	{
		old = map[x];
		map[x] |= map[x] << 1 | map[x] >> 1;
		map[x] &= edg[x];

	} while (map[x] != old);
}
void fill_rows_cmx(bin_img_t &map, bin_img_t &edg, size_t ctrX)
{
	for (size_t i = ctrX + 1; i < Xmax; ++i)
	{
		map[i] |= map[i - 1] & edg[i];
		fill_row(map, edg, i);
	}
}
void fill_rows_cmn(bin_img_t &map, bin_img_t &edg, size_t ctrX)
{
	for (size_t i = ctrX - 1; i != -1; --i)
	{
		map[i] |= map[i + 1] & edg[i];
		fill_row(map, edg, i);
	}
}
void fill_rows_mxc(bin_img_t &map, bin_img_t &edg, size_t ctrX)
{
	for (size_t i = Xmax - 2; i >= ctrX; --i)
	{
		map[i] |= map[i + 1] & edg[i];
		fill_row(map, edg, i);
	}
}
void fill_rows_mnc(bin_img_t &map, bin_img_t &edg, size_t ctrX)
{
	for (size_t i = 1; i <= ctrX; ++i)
	{
		map[i] |= map[i - 1] & edg[i];
		fill_row(map, edg, i);
	}
}
