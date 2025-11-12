#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "driver/gpio.h"

#define LED_GPIO       GPIO_NUM_4   // Changed LED pin (other LED)

static const char *TAG = "BLINK2";

static void blink_task(void *pvParameter)
{
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    while (1) {
        gpio_set_level(LED_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(500));  // blink faster for visual confirmation
        gpio_set_level(LED_GPIO, 0);
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());

    xTaskCreate(blink_task, "blink_task", 2048, NULL, 5, NULL);
    ESP_LOGI(TAG, "BLINK2 firmware running on new partition!");
}
