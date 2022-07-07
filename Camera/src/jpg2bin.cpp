
#include "jpg2bin.h"

#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>

#include <stddef.h>
#include <string.h>
// #include "img_converters.h"
#include <soc/efuse_reg.h>
#include <esp_heap_caps.h>
#include <sdkconfig.h>

#if ESP_IDF_VERSION_MAJOR >= 4 // IDF 4+
#if CONFIG_IDF_TARGET_ESP32	   // ESP32/PICO-D4
#include <esp32/rom/tjpgd.h>
#elif CONFIG_IDF_TARGET_ESP32S2
#include <tjpgd.h>
#elif CONFIG_IDF_TARGET_ESP32S3
#include <esp32s3/rom/tjpgd.h>
#else
#error Target CONFIG_IDF_TARGET is not supported
#endif
#else // ESP32 Before IDF 4.0
#include <rom/tjpgd.h>
#endif

// #if defined(ARDUINO_ARCH_ESP32) && defined(CONFIG_ARDUHAL_ESP_LOG)
// #include "esp32-hal-log.h"
// #define TAG ""
// #else
#include <esp_log.h>
static const char *TAG = "jpg2bin";
// #endif

typedef struct
{
	uint16_t width;
	uint16_t height;
	uint16_t data_offset;
	const uint8_t *input;
	uint8_t *output;
} rgb_jpg_decoder;

static void *_malloc(size_t size)
{
	return heap_caps_malloc(size, MALLOC_CAP_8BIT | MALLOC_CAP_INTERNAL); // MALLOC_CAP_INTERNAL MALLOC_CAP_SPIRAM
}

static uint32_t _jpg_read(void *arg, size_t index, uint8_t *buf, size_t len)
{
	rgb_jpg_decoder *jpeg = (rgb_jpg_decoder *)arg;
	if (buf)
	{
		memcpy(buf, jpeg->input + index, len);
	}
	return len;
}

static bool _bin_write(void *arg, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t *data)
{
	rgb_jpg_decoder *jpeg = (rgb_jpg_decoder *)arg;
	if (!data)
	{
		if (x == 0 && y == 0)
		{
			// write start
			jpeg->width = w;
			jpeg->height = h;
			// if output is null, this is BMP
			if (!jpeg->output)
			{
				jpeg->output = (uint8_t *)_malloc((w * h * 3) + jpeg->data_offset);
				if (!jpeg->output)
				{
					return false;
				}
			}
		}
		else
		{
			// write end
		}
		return true;
	}

	size_t jw = jpeg->width * 3;
	size_t jw2 = jpeg->width;
	size_t t = y * jw;
	size_t t2 = y * jw2;
	size_t bd = t + (h * jw);
	size_t l = x;
	uint8_t *out = jpeg->output + jpeg->data_offset;
	uint8_t *o = out;
	size_t iy, iy2, ix, ix2;

	w = w * 3;

	for (iy = t, iy2 = t2; iy < bd; iy += jw, iy2 += jw2)
	{
		o = out + iy2 + l;
		for (ix2 = ix = 0; ix < w; ix += 3, ix2 += 1)
		{
			uint8_t r = data[ix];
			uint8_t g = data[ix + 1];
			uint8_t b = data[ix + 2];
			// o[ix2] = (r / 3 + g / 3 + b / 3 + ((r % 3 + g % 3 + b % 3) / 3)) >> 1; // integer average
			o[ix2] = (r > g ? (r > b ? r : b) : (g > b ? g : b)) / 2; // integer max
		}
		data += w;
	}
	return true;
}

bool jpg2bin(const uint8_t *src, size_t src_len, uint8_t *out, jpg_scale_t scale)
{
	// ESP_LOGI(TAG, "jpg2bin called");
	rgb_jpg_decoder jpeg;
	jpeg.width = 0;
	jpeg.height = 0;
	jpeg.input = src;
	jpeg.output = out;
	jpeg.data_offset = 0;

	if (esp_jpg_decode(src_len, scale, _jpg_read, _bin_write, (void *)&jpeg) != ESP_OK)
	{
		return false;
	}
	return true;
}