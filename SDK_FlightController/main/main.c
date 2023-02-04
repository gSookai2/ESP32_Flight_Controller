#include <stdio.h>
#include "esp_system.h"

void app_main(void)
{
    printf("I am alive!");

    for (int i = 0; i < 10000000; i++);
    esp_restart();
}
