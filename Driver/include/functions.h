#pragma once

extern const char *TAG;

// Includes

// System
#include <esp_log.h>
#include <esp_system.h>

// I2C
#include <driver/i2c.h>

#include <esp_timer.h>

#include "gcem.hpp"

// =============

double GetTime()
{
	return (double)esp_timer_get_time() / 1000;
}

static inline int64_t get_time_us()
{
	return esp_timer_get_time();
}

// =============

static esp_err_t init_i2c()
{
	esp_err_t ret;

	i2c_config_t i2c_config = {
		.mode = I2C_MODE_SLAVE,
		.sda_io_num = GPIO_NUM_14,
		.scl_io_num = GPIO_NUM_13,
		.sda_pullup_en = GPIO_PULLUP_DISABLE,
		.scl_pullup_en = GPIO_PULLUP_DISABLE,
		.slave = {.addr_10bit_en = 0, .slave_addr = 0x67, .maximum_speed = 500000},
		.clk_flags = I2C_SCLK_SRC_FLAG_FOR_NOMAL,
	};

	ret = i2c_param_config(I2C_NUM_0, &i2c_config);
	if (ret != ESP_OK)
		return ret;

	ret = i2c_driver_install(I2C_NUM_0, I2C_MODE_SLAVE, 128, 128, ESP_INTR_FLAG_IRAM); //
	if (ret != ESP_OK)
		return ret;

	return ESP_OK;
}