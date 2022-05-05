
#include <esp_log.h>

#include "Controller.h"

static const char *TAG = "Controller";

Controller::Controller(Motor *mtr, ESP32Encoder *enc) : motor(mtr), encoder(enc)
{
	const esp_timer_create_args_t periodic_timer_args = {
		.callback = &Controller::callback,
		.arg = this,
		.dispatch_method = ESP_TIMER_TASK,
		.name = "ControllerTimer",
		.skip_unhandled_events = false,
	};

	ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
}

Controller::~Controller()
{
	ESP_ERROR_CHECK(esp_timer_stop(periodic_timer));
	ESP_ERROR_CHECK(esp_timer_delete(periodic_timer));
}

void Controller::attachMotor(Motor *mtr)
{
	motor = mtr;
}

void Controller::attachEncoder(ESP32Encoder *enc)
{
	encoder = enc;
}

void Controller::start()
{
	last = esp_timer_get_time();
	encoder->resumeCount();
	encoder->clearCount();
	ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 111 * 1000 - 357)); // every 10ms
}
void Controller::stop()
{
	encoder->pauseCount();
	ESP_ERROR_CHECK(esp_timer_stop(periodic_timer));
	ESP_ERROR_CHECK(esp_timer_delete(periodic_timer));
}

// static

void Controller::callback(void *arg)
{
	Controller *ctrl = reinterpret_cast<Controller *>(arg);

	int64_t time_now = esp_timer_get_time();
	int64_t time_diff = time_now - ctrl->last;

	int64_t ticks_now = ctrl->encoder->getCount();
	ctrl->encoder->clearCount();

	double speed = static_cast<double>(ticks_now) * 1000 * 1000 / time_diff; // TPS

	ESP_LOGI(TAG, "Controller callback! T_diff = %lld us; V = %f TPS", time_diff, speed);

	ctrl->last = time_now;
}
