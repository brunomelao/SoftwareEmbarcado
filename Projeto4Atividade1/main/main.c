
#include <stdio.h>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include <esp_log.h> // Recursos de LOG

void vTask1(void *pvparameters);
void vTask2(void *pvparameters);
void vTask3(void *pvparameters);
void vTask4(void *pvparameters);

TaskHandle_t t1Handle,t2Handle,t3Handle,t4Handle;

uint16_t lim=15;

void app_main(void)
{
    xTaskCreatePinnedToCore(vTask1,"Task 1",4096, NULL, 2, &t1Handle, 0);
    xTaskCreatePinnedToCore(vTask2,"Task 2",4096, NULL, 2, &t2Handle, 0);
    xTaskCreatePinnedToCore(vTask3,"Task 3",4096, NULL, 2, &t3Handle, 1);
    xTaskCreatePinnedToCore(vTask4,"Task 4",4096, NULL, 2, &t4Handle, 1);

}

void vTask1(void *pvparameters)
{

    BaseType_t coreID;
    UBaseType_t stack;
    ESP_LOGI("TASK1","Task 1 inicializando...");
    uint16_t ii,jj,cnt=0;
    while(1)
    {

        coreID = xPortGetCoreID();
        stack= uxTaskGetStackHighWaterMark(t1Handle);
        ESP_LOGI("TASK1","Task 1 executando no core %d sobrando %d de stack...", coreID,stack);
        for(ii = 0; ii<lim; ii++){
            for(jj = 0; jj<50000 ; jj++){
            }
            
        }
        if(cnt < 10) 
        {
            cnt++;
        }

        if(cnt == 10){
            vTaskDelete(t2Handle);
            cnt=11;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
void vTask2(void *pvparameters)
{
    BaseType_t coreID;
    uint16_t ii,jj;
    UBaseType_t stack;
    ESP_LOGI("TASK2","Task 2 inicializando...");
    while(1)
    {
        coreID = xPortGetCoreID();
        stack= uxTaskGetStackHighWaterMark(t2Handle);
        ESP_LOGI("TASK2","Task 2 executando no core %d sobrando %d de stack...", coreID,stack);
        for(ii = 0; ii<lim; ii++){
            for(jj = 0; jj<50000 ; jj++){
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
void vTask3(void *pvparameters)
{
    BaseType_t coreID;
    uint16_t ii,jj,cnt=0;
    UBaseType_t stack;
    ESP_LOGI("TASK3","Task 3 inicializando...");
    while(1)
    {
        coreID = xPortGetCoreID();
        stack= uxTaskGetStackHighWaterMark(t3Handle);
        ESP_LOGI("TASK3","Task 3 executando no core %d sobrando %d de stack...", coreID,stack);
        for(ii = 0; ii<lim; ii++){
            for(jj = 0; jj<50000 ; jj++){
            }
        }
        if(cnt < 20) 
        {
            cnt++;
        }

        if(cnt == 20){
            vTaskDelete(t4Handle);
            cnt=21;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
void vTask4(void *pvparameters)
{
    BaseType_t coreID;
    uint16_t ii,jj;
    UBaseType_t stack;
    ESP_LOGI("TASK4","Task 4 inicializando...");
    while(1)
    {
        coreID = xPortGetCoreID();
        stack= uxTaskGetStackHighWaterMark(t4Handle);
        ESP_LOGI("TASK4","Task 4 executando no core %d sobrando %d de stack...", coreID,stack);
        for(ii = 0; ii<lim; ii++){
            for(jj = 0; jj<50000 ; jj++){
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}
