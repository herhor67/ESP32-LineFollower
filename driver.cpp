
#define LOG_LOCAL_LEVEL ESP_LOG_VERBOSE

// CPP headers
#include <string>

// ESP system
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include <string.h>

#include <driver/gpio.h>

// FreeRTOS
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// Interrupts
#include <esp_intr_alloc.h>

// Memory
#include <esp_heap_caps.h>
#include <esp_timer.h>

// Classes
#include "Motor.h"
//#include "Vehicle.h"
//
//

static const char *TAG = "[" __TIME__
                         "]Cntrllr🅱 ";

double GetTime()
{
    return (double)esp_timer_get_time() / 1000;
}

static void IRAM_ATTR interrupt_handler(void *arg)
{
    static Motor mtrtest;
    mtrtest.encoderUpdate(gpio_get_level(GPIO_NUM_36), gpio_get_level(GPIO_NUM_39));

    //	ESP_LOGI(TAG, "Encoder: %i", (int)arg);
}

#ifdef __cplusplus
extern "C"
#endif
    void
    app_main(void)
{

    ESP_LOGI(TAG, "Wstało!");

    PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[12], PIN_FUNC_GPIO);
    PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[13], PIN_FUNC_GPIO);

    PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[34], PIN_FUNC_GPIO);
    PIN_FUNC_SELECT(GPIO_PIN_MUX_REG[35], PIN_FUNC_GPIO);

    double startTime;
    uint32_t startTick;

    gpio_reset_pin(GPIO_NUM_12);
    gpio_reset_pin(GPIO_NUM_13);

    gpio_set_direction(GPIO_NUM_12, GPIO_MODE_OUTPUT);
    gpio_set_direction(GPIO_NUM_13, GPIO_MODE_OUTPUT);

    gpio_reset_pin(GPIO_NUM_34);
    gpio_reset_pin(GPIO_NUM_35);
    gpio_set_direction(GPIO_NUM_34, GPIO_MODE_INPUT);
    gpio_set_direction(GPIO_NUM_35, GPIO_MODE_INPUT);
    gpio_pullup_dis(GPIO_NUM_34);
    gpio_pullup_dis(GPIO_NUM_35);

    gpio_install_isr_service(ESP_INTR_FLAG_IRAM | ESP_INTR_FLAG_LEVEL3); // ESP_INTR_FLAG_EDGE

    gpio_set_intr_type(GPIO_NUM_34, GPIO_INTR_ANYEDGE);
    gpio_set_intr_type(GPIO_NUM_35, GPIO_INTR_ANYEDGE);

    gpio_isr_handler_add(GPIO_NUM_34, interrupt_handler, (void *)GPIO_NUM_34);
    gpio_isr_handler_add(GPIO_NUM_35, interrupt_handler, (void *)GPIO_NUM_35);

    while (1)
    {
        startTick = esp_log_timestamp();
        startTime = GetTime();

        // gpio_set_level(GPIO_NUM_12, 0);
        // gpio_set_level(GPIO_NUM_13, 1);

        // vTaskDelay(pdMS_TO_TICKS(1000));

        // gpio_set_level(GPIO_NUM_12, 1);
        // gpio_set_level(GPIO_NUM_13, 0);

        // vTaskDelay(pdMS_TO_TICKS(1000));

        ESP_LOGI(TAG, "Encoder: %i", mtrtest.ticks);

        //	ESP_LOGI(TAG, "Diff: %i ms, %lf ms", esp_log_timestamp() - startTick, GetTime() - startTime);

        // vTaskDelay(pdMS_TO_TICKS(1000));
    }

    return;
}
