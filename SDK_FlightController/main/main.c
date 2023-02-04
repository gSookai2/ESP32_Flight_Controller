#include <stdio.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "esp_system.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "driver/mcpwm_prelude.h"

static const char* TAG = "SDK_FlightController";

#define SERVO_MIN_PULSEWIDTH_US 500  // Minimum pulse width in microsecond
#define SERVO_MAX_PULSEWIDTH_US 2500  // Maximum pulse width in microsecond
#define SERVO_MIN_DEGREE        -90   // Minimum angle
#define SERVO_MAX_DEGREE        90    // Maximum angle

static inline uint32_t example_angle_to_compare(int angle)
{
    return (angle - SERVO_MIN_DEGREE) * (SERVO_MAX_PULSEWIDTH_US - SERVO_MIN_PULSEWIDTH_US) / (SERVO_MAX_DEGREE - SERVO_MIN_DEGREE) + SERVO_MIN_PULSEWIDTH_US;
}

enum motorConstants
{
    resolution = 1000000,
    period = 10000
};

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
    //ESP_ERROR_CHECK(esp_timer_start_periodic(periodicTimer, 3000000));

    /* Create the PWM motor controller
        1. Create timer and operator
        2. Connect timer and operator
        3. Create comparator and generator from the operator
        4. Set generator action on timer and compare event
        5. Enable and start timer
    */

    // 1. Create timer and operator
    mcpwm_timer_handle_t motorTimer = NULL;
    mcpwm_timer_config_t motorConfig =
    {
        .group_id = 0,
        .clk_src = MCPWM_TIMER_CLK_SRC_DEFAULT,
        .resolution_hz = resolution,
        .count_mode = MCPWM_TIMER_COUNT_MODE_UP,
        .period_ticks = period
    };
    ESP_ERROR_CHECK(mcpwm_new_timer(&motorConfig, &motorTimer));

    mcpwm_oper_handle_t motorPWMOperator = NULL;
    mcpwm_operator_config_t motorPWMOperatorConfig =
    {
        .group_id = 0,
        .flags.update_gen_action_on_tep = 1
    };
    ESP_ERROR_CHECK(mcpwm_new_operator(&motorPWMOperatorConfig, &motorPWMOperator));

    // 2. Connect timer and operator
    ESP_ERROR_CHECK(mcpwm_operator_connect_timer(motorPWMOperator, motorTimer));

    // 3. Create the comparator and generator from the operator
    mcpwm_cmpr_handle_t motorPWMComparator = NULL;
    mcpwm_comparator_config_t motorPWMComparatorConfig =
    {
        .flags.update_cmp_on_tep = 1
    };
    ESP_ERROR_CHECK(mcpwm_new_comparator(motorPWMOperator, &motorPWMComparatorConfig, &motorPWMComparator));

    mcpwm_gen_handle_t motorPWMGenerator = NULL;
    mcpwm_generator_config_t motorPWMGeneratorConfig =
    {
        .gen_gpio_num = 15,
    };
    ESP_ERROR_CHECK(mcpwm_new_generator(motorPWMOperator, &motorPWMGeneratorConfig, &motorPWMGenerator));

    // 4. Set generator action on timer and compare event

    // go high on counter empty
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_timer_event(motorPWMGenerator,
                    MCPWM_GEN_TIMER_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, MCPWM_TIMER_EVENT_EMPTY, MCPWM_GEN_ACTION_HIGH)));
    // go low on compare threshold
    ESP_ERROR_CHECK(mcpwm_generator_set_action_on_compare_event(motorPWMGenerator,
                    MCPWM_GEN_COMPARE_EVENT_ACTION(MCPWM_TIMER_DIRECTION_UP, motorPWMComparator, MCPWM_GEN_ACTION_LOW)));

    // 5. Enable and start timer
    ESP_ERROR_CHECK(mcpwm_timer_enable(motorTimer));
    ESP_ERROR_CHECK(mcpwm_timer_start_stop(motorTimer, MCPWM_TIMER_START_NO_STOP));

    // Loop from example to check motor angle
    int angle = 0;
    int step = 2;
    while (1) {
        ESP_LOGI(TAG, "Angle of rotation: %d", angle);
        ESP_ERROR_CHECK(mcpwm_comparator_set_compare_value(motorPWMComparator, example_angle_to_compare(angle)));
        //Add delay, since it takes time for servo to rotate, usually 200ms/60degree rotation under 5V power supply
        vTaskDelay(pdMS_TO_TICKS(500));
        if ((angle + step) > 60 || (angle + step) < -60) {
            step *= -1;
        }
        angle += step;
    }
}
