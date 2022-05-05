
#include <esp_log.h>

#include "Buzzer.h"

static const char *TAG = "Buzzer";

Buzzer::Buzzer(ledc_timer_t timer, ledc_channel_t channel, gpio_num_t pin) : timer(timer), channel(channel), pin(pin)
{
	const esp_timer_create_args_t periodic_timer_args = {
		.callback = &Buzzer::callback,
		.arg = this,
		.dispatch_method = ESP_TIMER_TASK,
		.name = "BuzzerTimer",
		.skip_unhandled_events = false,
	};

	ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
}

Buzzer::~Buzzer()
{
	ESP_ERROR_CHECK(esp_timer_stop(periodic_timer));
	ESP_ERROR_CHECK(esp_timer_delete(periodic_timer));
}

void Buzzer::init()
{
	// Prepare and then apply the LEDC PWM timer configuration
	ledc_timer_config_t ledc_timer = {
		.speed_mode = ledc_mode,
		.duty_resolution = duty_res,
		.timer_num = timer,
		.freq_hz = 1000,
		.clk_cfg = LEDC_AUTO_CLK,
	};
	ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

	// Prepare and then apply the LEDC PWM channel configuration
	ledc_channel_config_t ledc_channel = {
		.gpio_num = pin,
		.speed_mode = ledc_mode,
		.channel = channel,
		.intr_type = LEDC_INTR_DISABLE,
		.timer_sel = timer,
		.duty = duty,
		.hpoint = 0,
	};
	ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

void Buzzer::loadSong(note_t *nts, duration_t *drtns, size_t lngth)
{
	notes = nts;
	durations = drtns;
	length = lngth;
}

void Buzzer::volumeOFF()
{
	ledc_set_duty(ledc_mode, channel, 0);
	ledc_update_duty(ledc_mode, channel);
	// ledc_set_duty_and_update(ledc_mode, channel, 0, 0);
}

void Buzzer::volumeON()
{
	ledc_set_duty(ledc_mode, channel, duty);
	ledc_update_duty(ledc_mode, channel);
	// ledc_set_duty_and_update(ledc_mode, channel, duty, 0);
}

void Buzzer::play(bool rpt)
{
	repeat = rpt;
	paused = false;
	currentNote = 0;

	volumeON();
	esp_timer_start_once(periodic_timer, 1);
}

void Buzzer::pause()
{
	paused = true;
	volumeOFF();
}

void Buzzer::resume()
{
	paused = false;

	volumeON();
	esp_timer_start_once(periodic_timer, 1);
}

// static

void Buzzer::callback(void *arg)
{
	Buzzer *bzzr = reinterpret_cast<Buzzer *>(arg);

	if (bzzr->paused)
		return;

	size_t n = ++bzzr->currentNote;

	if (n >= bzzr->length)
	{
		bzzr->currentNote = n = 0;
		if (!bzzr->repeat)
		{
			bzzr->pause();
			return;
		}
	}

	ESP_LOGI(TAG, "Buzzer callback! Note = %i, f = %i", n, bzzr->notes[n]);

	ledc_set_freq(bzzr->ledc_mode, bzzr->timer, bzzr->notes[n]);
	esp_timer_start_once(bzzr->periodic_timer, bzzr->durations[n]);
}
