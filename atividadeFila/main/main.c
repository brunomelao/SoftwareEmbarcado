#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include <freertos/queue.h>
#include <dht11.h>

QueueHandle_t queueTemp;
QueueHandle_t queueUmid;
QueueHandle_t queueMediaTemp;
QueueHandle_t queueMediaUmid;


struct dht11_reading data;

void vTask_Coleta(void* pvParameters);
void vTask_Display(void* pvParameters);
void vTask_MediaTemp(void* pvParameters);
void vTask_MediaUmid(void* pvParameters);

SemaphoreHandle_t sem_Cont;
SemaphoreHandle_t binTemp;
SemaphoreHandle_t binUmid;

void app_main(void)
{
   
    queueTemp = xQueueCreate(10,sizeof(int));
    queueUmid = xQueueCreate(10,sizeof(int));
    queueMediaTemp = xQueueCreate(10,sizeof(float));
    queueMediaUmid = xQueueCreate(10,sizeof(float));
    
    DHT11_init(DHT11_PIN);

    binTemp = xSemaphoreCreateBinary();
    binUmid = xSemaphoreCreateBinary();
    sem_Cont = xSemaphoreCreateCounting(2,0);

    xTaskCreatePinnedToCore(vTask_Coleta,"Task Coleta", 2048, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(vTask_MediaTemp,"Task Média Temperatura", 2048, NULL, 2, NULL, 1);
    xTaskCreatePinnedToCore(vTask_MediaUmid,"Task Média Umidade", 2048, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(vTask_Display,"Task Display", 2048, NULL, 1, NULL, 0);

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

void vTask_Display(void* pvParameters)
{
    ESP_LOGI("DISP","Task Display inicializando");
    float temperatura;
    float umidade;

    while (1)
    {
        if(uxSemaphoreGetCount(sem_Cont) == 2)
        {
            xQueueReceive(queueMediaTemp, &temperatura, portMAX_DELAY);
            xQueueReceive(queueMediaUmid, &umidade, portMAX_DELAY);
            ESP_LOGI("DISP","Media de Temperatura: %.2f", temperatura);
            ESP_LOGI("DISP","Media de Umidade: %.2f", umidade);
            xSemaphoreTake(sem_Cont, portMAX_DELAY);
            xSemaphoreTake(sem_Cont, portMAX_DELAY);
        }
    }
}
void vTask_MediaTemp(void* pvParameters)
{
    int dado, cont=1;
    float media=0;
    while(1)
    {
        
        xQueueReceive(queueTemp,&dado, portMAX_DELAY); 
        ESP_LOGW("TEMPS","Temp: %d",dado);  
        media = media + dado; 
        cont++;
        
        if(cont==10){
            media = media/10.0;
            xQueueSendToBack(queueMediaTemp, &media, portMAX_DELAY);
            media=0;
            cont=1;

        }
        
        xSemaphoreGive(sem_Cont);
    }
}

void vTask_MediaUmid(void* pvParameters)
{
    int dado, cont=1;
    float mediaUmid=0;
    while(1)
    {
        xQueueReceive(queueUmid,&dado, portMAX_DELAY);    
        ESP_LOGW("UMIDS","Umid: %d",dado);  

        mediaUmid = mediaUmid + dado; 
        cont++;
        
        if(cont==10){
            mediaUmid = mediaUmid/10.0;
            xQueueSendToBack(queueMediaUmid, &mediaUmid, portMAX_DELAY);
            mediaUmid=0;
            cont=1;
        }
        
        xSemaphoreGive(sem_Cont);
    }
}