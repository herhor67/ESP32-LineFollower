
#pragma once

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"

#include <esp_jpg_decode.h>

bool jpg2bin(const uint8_t *src, size_t src_len, uint8_t *out, jpg_scale_t scale);
