#include <stdio.h>
#include <string.h>

#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "freertos/semphr.h"
#include "freertos/message_buffer.h"
#include "driver/gpio.h"

#include "wifi.h"
#include "mqtt.h"

SemaphoreHandle_t wificonnectedSemaphore;
SemaphoreHandle_t mqttconnectedSemaphore;

#define BT_GPIO 0

void wifiConnected(void *params)
{
    while (1)
    {
        if (xSemaphoreTake(wificonnectedSemaphore, portMAX_DELAY))
        {
            mqtt_start();
        }
    }
}

void comunicacao_broker(void *params)
{
    char msg[50];

    xSemaphoreTake(mqttconnectedSemaphore, portMAX_DELAY);

    while (1)
    {
        float temp = 20.0 + (float)(rand() % 10);
        sprintf(msg, "temp = %f ALEX TELLESSSSSSSSSS", temp);
        ESP_LOGI("ENVIO", "%s", msg);
        if(gpio_get_level(BT_GPIO) == 0){
            ESP_LOGI("ENVIO","Botão pressionado");
            mqtt_publish("CEL080A/MSG/BancadaG", msg); // Trocar a letra da bancada para a letra da sua bancada
        }
        
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void app_main(void)
{
    gpio_reset_pin(BT_GPIO);
    gpio_set_direction(BT_GPIO, GPIO_MODE_INPUT);

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wificonnectedSemaphore = xSemaphoreCreateBinary();
    mqttconnectedSemaphore = xSemaphoreCreateBinary();

    wifi_start();

    xTaskCreate(wifiConnected, "Conexao MQTT", 4096, NULL, 2, NULL);
    xTaskCreate(comunicacao_broker, "Comunicacao com o Broker", 4096, NULL, 2, NULL);
}
