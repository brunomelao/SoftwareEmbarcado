#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <esp_log.h>

void vTaskS(void* pvparameters);
void vTaskW(void* pvparameters);

uint8_t cnt = 0;

SemaphoreHandle_t sem_sinc;
SemaphoreHandle_t sem_mutex;

void app_main(void)
{

    sem_sinc = xSemaphoreCreateBinary();
    sem_mutex = xSemaphoreCreateMutex();

    xTaskCreatePinnedToCore(vTaskS,"Signal", 2048, NULL, 1, NULL, 0);
    xTaskCreatePinnedToCore(vTaskW,"Wait", 2048, NULL, 2, NULL, 0);

}

void vTaskS(void* pvparameters)
{

    ESP_LOGI("SIGNAL", "Task inicializando");

    while(1){
        ESP_LOGI("SIGNAL", "Task executando");
        xSemaphoreTake(sem_mutex, portMAX_DELAY);
        cnt++;
        xSemaphoreGive(sem_mutex);
        xSemaphoreGive(sem_sinc);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void vTaskW(void* pvparameters)
{

    ESP_LOGI("WAIT", "Task inicializando");

    while(1)
    {
        ESP_LOGI("WAIT", "Tenta adquirir o semaforo");
        xSemaphoreTake(sem_sinc, portMAX_DELAY);
        xSemaphoreTake(sem_mutex, portMAX_DELAY);
        ESP_LOGI("WAIT", "O valor do contador e: %d", cnt);
        xSemaphoreGive(sem_mutex);

    }   
}
