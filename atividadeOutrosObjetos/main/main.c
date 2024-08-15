#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <driver/gpio.h>
#include <esp_log.h>
#include <dht11.h>
#include <freertos/event_groups.h>
#include <freertos/stream_buffer.h>

#define LIM_TEMP 22
#define LIM_UMID 50

#define EV_TEMP (1 << 0)
#define EV_UMID (1 << 1)
#define EV_ALARMET (1 << 2)
#define EV_ALARMEU (1 << 3)

EventGroupHandle_t ev_group;

struct dht11_reading data;

StreamBufferHandle_t bufferTemp;
StreamBufferHandle_t bufferUmid;
StreamBufferHandle_t bufferMediaTemp;
StreamBufferHandle_t bufferMediaUmid;

void vTaskLeitura(void* pvParameters);
void vTaskMediaTemp(void* pvParameters);
void vTaskMediaUmid(void* pvParameters);
void vTaskDisplay(void* pvParameters);
void vTaskAlarme(void* pvParameters);

void app_main(void)
{
    DHT11_init(DHT11_PIN);

    ev_group = xEventGroupCreate();

    bufferTemp = xStreamBufferCreate(50,10);
    bufferUmid = xStreamBufferCreate(50,10);
    bufferMediaTemp = xStreamBufferCreate(50,10);
    bufferMediaUmid = xStreamBufferCreate(50,10);

    xTaskCreate(vTaskLeitura, "Task Leitura", 2048, NULL, 1, NULL);
    xTaskCreate(vTaskMediaTemp, "Task Media Temp", 2048, NULL, 1, NULL);
    xTaskCreate(vTaskMediaUmid, "Task Media Umid", 2048, NULL, 1, NULL);
    xTaskCreate(vTaskDisplay, "Task Display", 2048, NULL, 1, NULL);
    xTaskCreate(vTaskAlarme, "Task Alarme", 2048, NULL, 1, NULL);
}

void vTaskLeitura(void* pvParameters)
{
    while(1)
    {
        data = DHT11_read();
         ESP_LOGI("LEITURA", "Dados enviados: Temperatura = %d , Umidade = %d.", data.temperature, data.humidity); 
        if(data.temperature > LIM_TEMP){
            xEventGroupSetBits(ev_group, EV_ALARMET);

        }
        if(data.humidity > LIM_UMID){
            xEventGroupSetBits(ev_group, EV_ALARMEU);

        }
        
        xStreamBufferSend(bufferTemp, &data.temperature, sizeof(data.temperature), portMAX_DELAY);
        xStreamBufferSend(bufferUmid, &data.humidity, sizeof(data.humidity), portMAX_DELAY);
       
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void vTaskMediaTemp(void* pvParameters)
{
    int dado, cont=0;
    float media=0;
    while(1)
    {
        xStreamBufferReceive(bufferTemp, &dado, sizeof(dado), portMAX_DELAY);
          
        media = media + (float) dado; 
        cont++;
        
        if(cont==10){
            media = media/10.0;
            xStreamBufferSend(bufferMediaTemp, &media, sizeof(float), portMAX_DELAY);
            ESP_LOGW("MEDIATEMP","Media Temperatura enviada: %.3f",media);
            xEventGroupSetBits(ev_group, EV_TEMP);
            media=0;
            cont=0;

        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}
void vTaskMediaUmid(void* pvParameters)
{
    int dado, cont=0;
    float media=0;
    while(1)
    {
        xStreamBufferReceive(bufferUmid, &dado, sizeof(dado), portMAX_DELAY);
         
        media = media + (float)dado; 
        cont++;
        
        if(cont==10){
            media = media/10.0;
            xStreamBufferSend(bufferMediaUmid, &media, sizeof(float), portMAX_DELAY);
            ESP_LOGW("MEDIAUMID","Media Umidade enviada: %.3f",media); 
            xEventGroupSetBits(ev_group, EV_UMID);
            media=0;
            cont=0;

        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void vTaskDisplay(void* pvParameters)
{
    EventBits_t bits;
    float mediaTemp, mediaUmid;
    while(1)
    {
        bits = xEventGroupWaitBits(ev_group, (EV_TEMP | EV_UMID), pdTRUE, pdTRUE, portMAX_DELAY);
        xStreamBufferReceive(bufferMediaTemp, &mediaTemp, sizeof(mediaTemp), portMAX_DELAY);
        xStreamBufferReceive(bufferMediaUmid, &mediaUmid, sizeof(mediaUmid), portMAX_DELAY);

        if (bits == (EV_TEMP | EV_UMID))
        {
            ESP_LOGI("DISPLAY", "Media Temperatura: %.3f ; Media Umidade: %.3f .", mediaTemp, mediaUmid);
        }

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void vTaskAlarme(void* pvParameters)
{
    EventBits_t bits;
    while(1)
    {
        bits = xEventGroupWaitBits(ev_group, (EV_ALARMET | EV_ALARMEU), pdTRUE, pdFALSE, portMAX_DELAY);

        if (bits == EV_ALARMET)
        {
            ESP_LOGE("ALARME", "Temperatura acima do limite!");
        }
        if (bits == EV_ALARMEU)
        {
            ESP_LOGE("ALARME", "Umidade acima do limite!");
        }
        if (bits == (EV_ALARMET | EV_ALARMEU))
        {
            ESP_LOGE("ALARME", "Temperatura e Umidade acima do limite!!");
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}