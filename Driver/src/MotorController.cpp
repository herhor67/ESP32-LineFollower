
#include <esp_log.h>

#include "MotorController.h"

static const char *TAG = "MtrCtrlr";

MotorController::MotorController(Motor *mtr, ESP32Encoder *enc, Controller<> *ctr) : motor(mtr), encoder(enc), controller(ctr)
{
	const esp_timer_create_args_t periodic_timer_args = {
		.callback = &MotorController::callback,
		.arg = this,
		.dispatch_method = ESP_TIMER_TASK,
		.name = "ControllerTimer",
		.skip_unhandled_events = true,
	};

	ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
}

MotorController::~MotorController()
{
	ESP_ERROR_CHECK(esp_timer_stop(periodic_timer));
	ESP_ERROR_CHECK(esp_timer_delete(periodic_timer));
}

void MotorController::attachMotor(Motor *mtr)
{
	motor = mtr;
}
void MotorController::attachEncoder(ESP32Encoder *enc)
{
	encoder = enc;
}
void MotorController::attachController(Controller<> *ctr)
{
	controller = ctr;
}

void MotorController::setSetpoint(float sp)
{
	setpoint = sp;
}

void MotorController::start()
{
	time_last = esp_timer_get_time();
	tick_last = encoder->getCount();
	curr_speed = 0.0 / 0.0;
	// encoder->resumeCount();
	// encoder->clearCount();
	ESP_ERROR_CHECK(esp_timer_start_periodic(periodic_timer, 10 * 1000)); // every 10ms
}
void MotorController::stop()
{
	// encoder->pauseCount();
	ESP_ERROR_CHECK(esp_timer_stop(periodic_timer));
	ESP_ERROR_CHECK(esp_timer_delete(periodic_timer));
	motor->stop();
}

// static

void MotorController::callback(void *arg)
{
	MotorController *MC = reinterpret_cast<MotorController *>(arg);

	int64_t time_now = esp_timer_get_time();
	int64_t time_diff = time_now - MC->time_last;
	MC->time_last = time_now;

	int64_t tick_now = MC->encoder->getCount();
	int64_t tick_diff = tick_now - MC->tick_last;
	MC->tick_last = tick_now;

	float dt = time_diff / 1e6f; // seconds
	float dxdt = tick_diff / dt; // TPS

	if (std::isnan(MC->curr_speed))
		MC->curr_speed = dxdt;

	float rmmbrfctr = std::exp(-dt * 5); //  / 1e6f
	MC->curr_speed = rmmbrfctr * MC->curr_speed + (1 - rmmbrfctr) * dxdt;

	// MC->updateSetpoint();

	MC->steering = MC->controller->updateO({MC->setpoint, MC->curr_speed}, dt);

	MC->motor->duty(MC->steering);

	if (!(MC->i = (MC->i + 1) % 10))
		ESP_LOGI(TAG, "tck = %lld; V = %f; O=%f; PID = %f %f %f", tick_diff, MC->curr_speed, MC->controller->O, MC->controller->P.O, MC->controller->I.O, MC->controller->D.O);
}
