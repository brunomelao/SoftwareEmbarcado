#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <esp_log.h>

QueueHandle_t queue1, queue2;
QueueSetHandle_t queueset;

void vTask1(void *pvparameters);
void vTask2(void *pvparameters);
void vTask3(void *pvparameters);


void app_main(void)
{
    queue1 = xQueueCreate(10, sizeof(uint16_t));
    queue2 = xQueueCreate(10, sizeof(uint16_t));

    queueset = xQueueCreateSet(2*10);

    xQueueAddToSet(queue1, queueset); 
    xQueueAddToSet(queue2, queueset); 

    xTaskCreate(vTask1, "Task 1", 2048, NULL, 1, NULL);
    xTaskCreate(vTask2, "Task 2", 2048, NULL, 1, NULL);
    xTaskCreate(vTask3, "Task 3", 2048, NULL, 1, NULL);
}

void vTask1(void *pvparameters)
{
    uint16_t cnt = 0;
    portBASE_TYPE status;
    while(1)
    {
        status = xQueueSendToBack(queue1, &cnt, 0);
        if (status == pdTRUE)
        {
            ESP_LOGI("TASK1", "Mensagem CNT = %d enviada corretamente", cnt);
        }
        else
        {
            ESP_LOGE("TASK1", "Mensagem CNT = %d perdida", cnt);
        }
        cnt += 2;
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
void vTask2(void *pvparameters)
{
    uint16_t cnt = 1;
    portBASE_TYPE status;
    while(1)
    {
        status = xQueueSendToBack(queue2, &cnt, 0);
        if (status == pdTRUE)
        {
            ESP_LOGI("TASK2", "Mensagem CNT = %d enviada corretamente", cnt);
        }
        else
        {
            ESP_LOGE("TASK2", "Mensagem CNT = %d perdida", cnt);
        }
        cnt += 2;
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
void vTask3(void *pvparameters)
{
    uint16_t msg;
    QueueSetHandle_t queue_data;
    
    while(1)
    {
        queue_data = xQueueSelectFromSet(queueset, portMAX_DELAY);


        xQueueReceive(queue_data, &msg, 0);        

        if (queue_data==queue1)
        {
            ESP_LOGW("TASK3","Mensagem MSG = %d recebida da Queue1.", msg);
        }
        else
        {
            ESP_LOGW("TASK3","Mensagem MSG = %d recebida da Queue2.", msg);
        }
    }
}

