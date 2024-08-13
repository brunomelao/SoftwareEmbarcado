#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>

void vTask(void* pvParameters);


void app_main(void)
{
    xTaskCreatePinnedToCore(vTask, "Task 1", 2048, 1000, 1, NULL, 0);
    xTaskCreatePinnedToCore(vTask, "Task 2", 2048, 5000, 1, NULL, 1);
}

void vTask(void* pvParameters)
{
    BaseType_t core;
    char* name = pcTaskGetName(NULL);
    uint16_t ms_delay =(uint16_t)pvParameters; 
    ESP_LOGE("TASK","%s Inicializando", name);
    while(1)
    {
        core = xPortGetCoreID();
        ESP_LOGI("TASK","%s executando no core %d", name, core);
        vTaskDelay(pdMS_TO_TICKS(ms_delay));
    }
}