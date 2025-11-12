// #include <string.h>
// #include "freertos/FreeRTOS.h"
// #include "freertos/task.h"
// #include "esp_wifi.h"
// #include "esp_event.h"
// #include "esp_log.h"
// #include "nvs_flash.h"
// #include "esp_ota_ops.h"
// #include "esp_http_client.h"
// #include "driver/gpio.h"
// #include "esp_https_ota.h"

// #define WIFI_SSID      "sl4y3r07"
// #define WIFI_PASS      "ayush231"
// #define OTA_URL        "http://10.125.136.88:8000/ota_blink.bin"  // OTA binary URL

// #define LED_GPIO       GPIO_NUM_2   // On-board LED for most boards

// static const char *TAG = "OTA_BLINK";

// // Blink task
// static void blink_task(void *pvParameter)
// {
//     gpio_reset_pin(LED_GPIO);
//     gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
//     while (1) {
//         gpio_set_level(LED_GPIO, 1);
//         vTaskDelay(pdMS_TO_TICKS(1000));
//         gpio_set_level(LED_GPIO, 0);
//         vTaskDelay(pdMS_TO_TICKS(1000));
//     }
// }

// // Wi-Fi event handler
// static void wifi_event_handler(void *arg, esp_event_base_t event_base,
//                                int32_t event_id, void *event_data)
// {
//     if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
//         esp_wifi_connect();
//     } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
//         esp_wifi_connect();
//         ESP_LOGI(TAG, "Retrying connection...");
//     } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
//         ESP_LOGI(TAG, "Connected to Wi-Fi!");
//     }
// }

// // Wi-Fi init
// static void wifi_init(void)
// {
//     esp_netif_init();
//     esp_event_loop_create_default();
//     esp_netif_create_default_wifi_sta();

//     wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
//     ESP_ERROR_CHECK(esp_wifi_init(&cfg));

//     esp_event_handler_instance_t instance_any_id;
//     esp_event_handler_instance_t instance_got_ip;
//     ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
//                                         ESP_EVENT_ANY_ID,
//                                         &wifi_event_handler,
//                                         NULL,
//                                         &instance_any_id));
//     ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
//                                         IP_EVENT_STA_GOT_IP,
//                                         &wifi_event_handler,
//                                         NULL,
//                                         &instance_got_ip));

//     wifi_config_t wifi_config = {
//         .sta = {
//             .ssid = WIFI_SSID,
//             .password = WIFI_PASS,
//             .threshold.authmode = WIFI_AUTH_WPA2_PSK,
//         },
//     };
//     ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
//     ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
//     ESP_ERROR_CHECK(esp_wifi_start());

//     ESP_LOGI(TAG, "Connecting to Wi-Fi...");
// }

// // OTA task
// static void ota_task(void *pvParameter)
// {
//     ESP_LOGI(TAG, "Starting OTA from: %s", OTA_URL);

//     // HTTP client config
//     esp_http_client_config_t http_config = {
//         .url = OTA_URL,
//         .timeout_ms = 5000,
//         .skip_cert_common_name_check = true,
//     };

//     // // OTA config
//     // esp_https_ota_config_t ota_config = {
//     //     .http_config = &http_config,
        
//     // };

//     // // Perform OTA
//     // esp_err_t ret = esp_https_ota(&ota_config);

// //     esp_http_client_config_t http_config = {
// //     .url = OTA_URL,
// //     .timeout_ms = 5000,
// //     .cert_pem = NULL, // <--- this tells the OTA client it’s HTTP/insecure
// // };

// esp_https_ota_config_t ota_config = {
//     .http_config = &http_config 
// };

// esp_err_t ret = esp_https_ota(&ota_config);

//     if (ret == ESP_OK) {
//         ESP_LOGI(TAG, "OTA Successful! Rebooting...");
//         esp_restart();
//     } else {
//         ESP_LOGE(TAG, "OTA Failed!");
//     }

//     vTaskDelete(NULL);
// }

// // app_main
// void app_main(void)
// {
//     ESP_ERROR_CHECK(nvs_flash_init());
//     wifi_init();

//     xTaskCreate(blink_task, "blink_task", 2048, NULL, 5, NULL);

//     // Wait for Wi-Fi connection before OTA
//     vTaskDelay(pdMS_TO_TICKS(10000));

//     xTaskCreate(ota_task, "ota_task", 8192, NULL, 5, NULL);
// }


#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_ota_ops.h"
#include "esp_http_client.h"
#include "driver/gpio.h"
#include "esp_https_ota.h"

#define WIFI_SSID      "sl4y3r07"
#define WIFI_PASS      "ayush231"
#define OTA_URL        "http://10.114.233.88:8000/ota_blink.bin"  

#define LED_GPIO       GPIO_NUM_2   

static const char *TAG = "OTA_BLINK";

// Blink task
static void blink_task(void *pvParameter)
{
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    while (1) {
        gpio_set_level(LED_GPIO, 1);
        vTaskDelay(pdMS_TO_TICKS(1000));
        gpio_set_level(LED_GPIO, 0);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

// Wi-Fi event handler
static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        esp_wifi_connect();
        ESP_LOGI(TAG, "Retrying connection...");
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(TAG, "Connected to Wi-Fi!");
    }
}

// Wi-Fi init
static void wifi_init(void)
{
    esp_netif_init();
    esp_event_loop_create_default();
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                        ESP_EVENT_ANY_ID,
                                        &wifi_event_handler,
                                        NULL,
                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                        IP_EVENT_STA_GOT_IP,
                                        &wifi_event_handler,
                                        NULL,
                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Connecting to Wi-Fi...");
}

// OTA task
static void ota_task(void *pvParameter)
{
    ESP_LOGI(TAG, "Starting OTA from: %s", OTA_URL);

    // HTTP client config
    esp_http_client_config_t http_config = {
        .url = OTA_URL,
        .timeout_ms = 5000,
        .skip_cert_common_name_check = true, // allows HTTP/insecure
    };

    esp_https_ota_config_t ota_config = {
        .http_config = &http_config
    };

    
    
    esp_http_client_handle_t client = esp_http_client_init(&http_config);
    esp_err_t err = esp_http_client_open(client, 0);
    ESP_LOGI(TAG, "esp_http_client_open() returned: %s", esp_err_to_name(err));
    esp_http_client_cleanup(client);
    
    ESP_LOGI(TAG, "Performing OTA...");
    esp_err_t ret = esp_https_ota(&ota_config);
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "OTA Successful! Rebooting...");
        esp_restart();
    } else {
        ESP_LOGE(TAG, "OTA Failed!");
    }

    // --- Check which partition is active and its state ---
    const esp_partition_t *running = esp_ota_get_running_partition();
    const esp_partition_t *boot = esp_ota_get_boot_partition();
    ESP_LOGI(TAG, "Running partition: %s at 0x%08x", running->label, running->address);
    ESP_LOGI(TAG, "Boot partition: %s at 0x%08x", boot->label, boot->address);

    esp_ota_img_states_t ota_state;
    if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK) {
        switch (ota_state) {    
            case ESP_OTA_IMG_NEW: ESP_LOGI(TAG, "New OTA image"); break;
            case ESP_OTA_IMG_PENDING_VERIFY: ESP_LOGI(TAG, "Pending verification"); break;
            case ESP_OTA_IMG_VALID: ESP_LOGI(TAG, "Valid OTA image"); break;
            case ESP_OTA_IMG_INVALID: ESP_LOGI(TAG, "Invalid OTA image"); break;
            default: ESP_LOGI(TAG, "Unknown OTA state"); break;
        }
    }

    vTaskDelete(NULL);
}

// app_main
void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    wifi_init();

    xTaskCreate(blink_task, "blink_task", 2048, NULL, 5, NULL);

    // Wait for Wi-Fi connection before OTA
    vTaskDelay(pdMS_TO_TICKS(10000));

    xTaskCreate(ota_task, "ota_task", 8192, NULL, 5, NULL);
}
