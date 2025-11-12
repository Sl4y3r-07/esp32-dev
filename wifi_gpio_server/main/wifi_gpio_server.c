#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "esp_http_server.h"
#include "driver/gpio.h"

#define LED_GPIO 2 
#define WIFI_SSID "sl4y307" 
#define WIFI_PASS "ayush231"

static const char *TAG = "WIFI_GPIO_SERVER";
static httpd_handle_t server = NULL;


static esp_err_t root_get_handler(httpd_req_t *req)
{
    const char *resp =
        "<!DOCTYPE html><html><head><title>ESP32 GPIO Control</title>"
        "<meta name='viewport' content='width=device-width, initial-scale=1'/>"
        "<style>body{font-family:sans-serif;text-align:center;}button{font-size:20px;padding:10px 25px;margin:10px;}</style>"
        "</head><body>"
        "<h2>ESP32 GPIO 2 Control</h2>"
        "<p><button onclick=\"location.href='/on'\">Power ON</button>"
        "<button onclick=\"location.href='/off'\">Power OFF</button></p>"
        "<p><a href='/'>Refresh</a></p>"
        "</body></html>";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

static esp_err_t gpio_on_handler(httpd_req_t *req)
{
    gpio_set_level(LED_GPIO, 1);
    ESP_LOGI(TAG, "GPIO %d -> ON", LED_GPIO);
    httpd_resp_sendstr(req, "GPIO 2 Powered ON<br><a href='/'>Back</a>");
    return ESP_OK;
}

static esp_err_t gpio_off_handler(httpd_req_t *req)
{
    gpio_set_level(LED_GPIO, 0);
    ESP_LOGI(TAG, "GPIO %d -> OFF", LED_GPIO);
    httpd_resp_sendstr(req, "GPIO 2 Powered OFF<br><a href='/'>Back</a>");
    return ESP_OK;
}

// ==== WEB SERVER ==== //

static httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK)
    {
        httpd_uri_t root_uri = { .uri="/", .method=HTTP_GET, .handler=root_get_handler };
        httpd_uri_t on_uri   = { .uri="/on", .method=HTTP_GET, .handler=gpio_on_handler };
        httpd_uri_t off_uri  = { .uri="/off", .method=HTTP_GET, .handler=gpio_off_handler };
        httpd_register_uri_handler(server, &root_uri);
        httpd_register_uri_handler(server, &on_uri);
        httpd_register_uri_handler(server, &off_uri);
    }
    return server;
}

// ==== WIFI HANDLER ==== //

static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        ESP_LOGI(TAG, "Wi-Fi disconnected, reconnecting...");
        esp_wifi_connect();
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));

        gpio_set_level(LED_GPIO, 1);
        ESP_LOGI(TAG, "Wi-Fi connected, GPIO %d -> ON", LED_GPIO);

        // Start web server
        if (server == NULL)
        {
            server = start_webserver();
            ESP_LOGI(TAG, "Web server started");
        }
    }
}


void wifi_init_sta(void)
{
    esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t any_id, got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, &any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, &got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
        },
    };

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "Connecting to Wi-Fi SSID:%s", WIFI_SSID);
}


void app_main(void)
{

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

 
    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);
    gpio_set_level(LED_GPIO, 0); // initially OFF


    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    wifi_init_sta();
}
