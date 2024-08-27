#include <stdio.h>
#include <string.h>

#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "freertos/semphr.h"
#include "freertos/message_buffer.h"

#include "wifi.h"
#include "http_client.h"

SemaphoreHandle_t wificonnectedSemaphore;

MessageBufferHandle_t buffer_http;

void RealizaHTTPRequest(void *params)
{
    char str[600];
    size_t bytes_recebidos;
    
    xSemaphoreTake(wificonnectedSemaphore, portMAX_DELAY);

    while (1)
    {
        ESP_LOGI("Task", "Realiza HTTP Request");
        http_request();

        bytes_recebidos = xMessageBufferReceive(buffer_http, str, sizeof(str), portMAX_DELAY);
        ESP_LOGW("TASK", "%s", str);

        vTaskDelay(pdMS_TO_TICKS(30000));
    }
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

    wificonnectedSemaphore = xSemaphoreCreateBinary();

    buffer_http = xMessageBufferCreate(600);

    wifi_start();

    xTaskCreate(RealizaHTTPRequest, "Processa HTTP", 4096, NULL, 2, NULL);
}
