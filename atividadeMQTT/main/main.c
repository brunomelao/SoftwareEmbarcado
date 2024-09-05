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
#include <dht11.h>

#include "wifi.h"
#include "mqtt.h"

SemaphoreHandle_t wificonnectedSemaphore;
SemaphoreHandle_t mqttconnectedSemaphore;

QueueHandle_t queueTemp;
QueueHandle_t queueUmid;
QueueHandle_t queueMediaTemp;
QueueHandle_t queueMediaUmid;

struct dht11_reading data;

void vTask_Coleta(void* pvParameters);
void vTask_Media(void* pvParameters);


SemaphoreHandle_t binTemp;
SemaphoreHandle_t binUmid;

int N = 10;
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
    char msgTemp[50];
    char msgUmid[50];

    xSemaphoreTake(mqttconnectedSemaphore, portMAX_DELAY);
    mqtt_sbscribe("CEL080B/Sensores/TamanhoMedia");
    float temperatura;
    float umidade;

    while (1)
    {
        xQueueReceive(queueMediaTemp, &temperatura, portMAX_DELAY);
        xQueueReceive(queueMediaUmid, &umidade, portMAX_DELAY);
        sprintf(msgTemp, "Media de Temperatura: %.2f ", temperatura);
        sprintf(msgUmid, "Media de Umidade: %.2f ", umidade);

        ESP_LOGI("ENVIO", "%s", msgTemp);
        ESP_LOGI("ENVIO", "%s", msgUmid);

       
        mqtt_publish("CEL080B/Sensores/BancadaG/Temperatura", msgTemp); // Trocar a letra da bancada para a letra da sua bancada
        mqtt_publish("CEL080B/Sensores/BancadaG/Umidade", msgUmid); // Trocar a letra da bancada para a letra da sua bancada

        
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void vTask_Coleta(void* pvParameters);
void vTask_Media(void* pvParameters);

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

    queueTemp = xQueueCreate(10,sizeof(int));
    queueUmid = xQueueCreate(10,sizeof(int));
    queueMediaTemp = xQueueCreate(10,sizeof(float));
    queueMediaUmid = xQueueCreate(10,sizeof(float));
    
    DHT11_init(DHT11_PIN);

    binTemp = xSemaphoreCreateBinary();
    binUmid = xSemaphoreCreateBinary();

    xTaskCreate(wifiConnected, "Conexao MQTT", 4096, NULL, 2, NULL);
    xTaskCreate(comunicacao_broker, "Comunicacao com o Broker", 4096, NULL, 2, NULL);
    xTaskCreate(vTask_Coleta, "Coleta",4096, NULL, 1,NULL);
    xTaskCreate(vTask_Media, "Media",4096, NULL, 1,NULL);

}

void vTask_Coleta(void* pvParameters)
{

    ESP_LOGI("COLETA","Task Coleta inicializando");

    while (1)
    {
        data = DHT11_read();
        if (uxQueueMessagesWaiting(queueTemp) == 0)
        {
            xSemaphoreGive(binTemp);
        }
        if (uxQueueMessagesWaiting(queueUmid) == 0)
        {
            xSemaphoreGive(binUmid);
        }
        xQueueSendToBack(queueTemp, &data.temperature, portMAX_DELAY);
        xQueueSendToBack(queueUmid, &data.humidity, portMAX_DELAY);
        vTaskDelay(pdMS_TO_TICKS(200));
    }  
}

void vTask_Media(void* pvParameters)
{
    int dadoUmid,dadoTemp, cont=0;
    float mediaTemp=0,mediaUmid=0;
    while(1)
    {
        
        xQueueReceive(queueTemp,&dadoTemp, portMAX_DELAY); 
        xQueueReceive(queueUmid,&dadoUmid, portMAX_DELAY);    

        ESP_LOGW("MEDIA","Temp: %d",dadoTemp);  
        ESP_LOGW("MEDIA","Umid: %d",dadoUmid);  

        mediaTemp = mediaTemp + dadoTemp; 
        mediaUmid = mediaUmid + dadoUmid; 
        cont++;
        if(cont==N){
            ESP_LOGE("MEDIA","N = %d",N);

            mediaTemp = mediaTemp/N;
            mediaUmid = mediaUmid/N;

            xQueueSendToBack(queueMediaTemp, &mediaTemp, portMAX_DELAY);
            xQueueSendToBack(queueMediaUmid, &mediaUmid, portMAX_DELAY);

            mediaTemp=0;
            mediaUmid=0;
            cont=0;
        }
        vTaskDelay(pdMS_TO_TICKS(300));

    }
}

