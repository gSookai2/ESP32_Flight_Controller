#include <stdio.h>
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_log.h"

static const char* TAG = "SDK_FlightController";

void timerCallback(void* arg)
{
    ESP_LOGI(TAG, "Timer test");
}

void app_main(void)
{
    // Create a periodic timer
    const esp_timer_create_args_t periodicConfig =
    {
        .callback = &timerCallback,
        .name = "Periodic"
    };

    esp_timer_handle_t periodicTimer;
    ESP_ERROR_CHECK(esp_timer_create(&periodicConfig, &periodicTimer));

    // Start the timer
    ESP_ERROR_CHECK(esp_timer_start_periodic(periodicTimer, 3000000));
}
