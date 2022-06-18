
#include <esp_log.h>

#include "Buzzer.h"

static const char *TAG = "Buzzer";

Buzzer::Buzzer(gpio_num_t pin, ledc_timer_t timer, ledc_channel_t channel) : pin(pin), timer(timer), channel(channel)
{
	// ESP high resolution timer
	const esp_timer_create_args_t periodic_timer_cfg = {
		.callback = &Buzzer::callback,
		.arg = this,
		.dispatch_method = ESP_TIMER_TASK,
		.name = "BuzzerTimer",
		.skip_unhandled_events = false,
	};
	esp_timer_create(&periodic_timer_cfg, &periodic_timer);

	// Prepare and then apply the LEDC PWM timer configuration
	ledc_timer_config_t ledc_timer_cfg = {
		.speed_mode = LEDCMODE,
		.duty_resolution = DUTYRES,
		.timer_num = timer,
		.freq_hz = 100,
		.clk_cfg = LEDC_AUTO_CLK,
	};
	ledc_timer_config(&ledc_timer_cfg);
	volumeOFF();

	// Prepare and then apply the LEDC PWM channel configuration
	ledc_channel_config_t ledc_channel_cfg = {
		.gpio_num = pin,
		.speed_mode = LEDCMODE,
		.channel = channel,
		.intr_type = LEDC_INTR_DISABLE,
		.timer_sel = timer,
		.duty = DUTY,
		.hpoint = 0,
	};
	ledc_channel_config(&ledc_channel_cfg);
}

Buzzer::~Buzzer()
{
	esp_timer_stop(periodic_timer);
	esp_timer_delete(periodic_timer);

	ledc_stop(LEDCMODE, channel, 0);
}

void Buzzer::setTempo(uint32_t BPM, Duration base)
{
	tempo = BPM * static_cast<duration_t>(base);
}

void Buzzer::loadSong(note_t *nts, duration_t *drtns, size_t lngth)
{
	notes = nts;
	durations = drtns;
	length = lngth;
}

void Buzzer::volumeOFF()
{
	ledc_timer_pause(LEDCMODE, timer);
	// ledc_set_duty(LEDCMODE, channel, 0);
	// ledc_update_duty(LEDCMODE, channel);
}

void Buzzer::volumeON()
{
	ledc_timer_resume(LEDCMODE, timer);
	// ledc_set_duty(LEDCMODE, channel, DUTY);
	// ledc_update_duty(LEDCMODE, channel);
}

void Buzzer::start(bool rpt)
{
	if (!midsong)
	{
		if (length > 0)
		{
			midsong = true;
			paused = false;
			repeat = rpt;
			currentNote = 0;
			remainingTime = -1;

			volumeON();
			playCurrentNote();
		}
	}
	else
		ESP_LOGW(TAG, "Cant start when in song!");
}

void Buzzer::end()
{
	if (midsong)
	{
		midsong = false;
		paused = false;

		volumeOFF();
		esp_timer_stop(periodic_timer);
	}
	else
		ESP_LOGW(TAG, "Cant end when not in song!");
}

void Buzzer::pause()
{
	if (midsong)
	{
		if (!paused)
		{
			paused = true;
			// esp_timer_get_expiry_time(periodic_timer, remainingTime);

			volumeOFF();
			esp_timer_stop(periodic_timer);
		}
		else
			ESP_LOGW(TAG, "Cant pause when not playing!");
	}
	else
		ESP_LOGW(TAG, "Cant pause when not in song!");
}

void Buzzer::resume()
{
	if (midsong)
	{
		if (paused)
		{
			paused = false;

			if (checkCurrentNote())
			{
				volumeON();
				playCurrentNote();
			}
		}
		else
			ESP_LOGW(TAG, "Cant resume when not paused!");
	}
	else
		ESP_LOGW(TAG, "Cant resume when not in song!");
}

void Buzzer::restart()
{
	if (midsong)
	{
		end();
		start(repeat);
	}
	else
		ESP_LOGW(TAG, "Cant restart when not in song!");
}

bool Buzzer::checkCurrentNote()
{
	if (currentNote < length)
		return true;

	end();
	start(repeat);

	return false;
}

void Buzzer::playCurrentNote()
{
	if (!checkCurrentNote())
		return;

	size_t n = currentNote;

	ESP_LOGI(TAG, "Buzzer callback! Note = %i, f = %i", n, notes[n]);
	ESP_LOGI(TAG, "WTF: D= %i, s = %u, t = %lld", durations[n], static_cast<duration_t>(Duration::second), tempo);

	ledc_set_freq(LEDCMODE, timer, notes[n]);

	if (remainingTime != -1)
	{
		esp_timer_start_once(periodic_timer, remainingTime);
		remainingTime = -1;
	}
	else
		esp_timer_start_once(periodic_timer, durations[n] * 60ull * static_cast<duration_t>(Duration::second) / tempo);
}

// static

void Buzzer::callback(void *arg)
{
	Buzzer *bzzr = reinterpret_cast<Buzzer *>(arg);

	size_t n = ++bzzr->currentNote;

	if (!bzzr->midsong || bzzr->paused)
		ESP_LOGW(TAG, "Buzzer callback but not midsong or paused! Note = %i, f = %i", bzzr->currentNote, bzzr->notes[n]);
	// return;

	bzzr->playCurrentNote();
}
