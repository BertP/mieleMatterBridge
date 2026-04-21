#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"

static const char *TAG = "SMOKE_TEST";

/* Nutze einen Standard-Pin für das DevKit (GPIO 2 oder 48 beim S3)
   Wenn dein Board einen anderen Pin hat, passen wir das an. */
#define BLINK_GPIO 2 

void app_main(void)
{
    ESP_LOGI(TAG, "Toolchain Smoke Test gestartet!");
    ESP_LOGI(TAG, "Hardware: ESP32-S3");
    
    gpio_reset_pin(BLINK_GPIO);
    gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT);

    while (1) {
        ESP_LOGI(TAG, "Blink...");
        gpio_set_level(BLINK_GPIO, 1);
        vTaskDelay(500 / portTICK_PERIOD_MS);
        gpio_set_level(BLINK_GPIO, 0);
        vTaskDelay(500 / portTICK_PERIOD_MS);
    }
}
