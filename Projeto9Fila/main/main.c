#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include <freertos/queue.h>

#define BT_GPIO 0
#define LED_GPIO 2

QueueHandle_t myqueue;

struct mystruct
{
    uint16_t cnt;
    TickType_t ticks;
};

void vTask_BT(void* pvParameters);
void vTask_DISP(void* pvParameters);

void app_main(void)
{
    gpio_reset_pin(BT_GPIO);
    gpio_set_direction(BT_GPIO, GPIO_MODE_INPUT);

    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    myqueue = xQueueCreate(5, sizeof(struct mystruct));
    
    xTaskCreatePinnedToCore(vTask_BT,"Task Botão", 2048, NULL, 1, NULL, 1);
    xTaskCreatePinnedToCore(vTask_DISP,"Task Disp", 2048, NULL, 1, NULL, 0);
    
}

void vTask_BT(void* pvParameters)
{
    struct mystruct btstruct;
    btstruct.cnt = 0;
     
    ESP_LOGI("BT","Task Botão inicializando");

    while (1)
    {
        if(gpio_get_level(BT_GPIO) == 0){
            btstruct.cnt++;
            btstruct.ticks = xTaskGetTickCount();

            ESP_LOGI("TaskBT", "Botão pressionado. CNT = %d.", btstruct.cnt);
            xQueueSendToBack(myqueue, &btstruct, portMAX_DELAY);
            ESP_LOGI("BT","Mensagem enviada!");

        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }  
}

void vTask_DISP(void* pvParameters)
{
    struct mystruct dispstruct;


    ESP_LOGI("DISP","Task Display inicializando");

    while (1)
    {
        /*
        if (uxQueueMessagesWaiting(myqueue) == 0)
        {
            vTaskDelay(pdMS_TO_TICKS(10000));
        }
        */

        xQueueReceive(myqueue, &dispstruct, portMAX_DELAY);
        ESP_LOGE("DISP","Dado recebido. CNT = %d. Botão pressionado em %d ms", dispstruct.cnt, (int)pdMS_TO_TICKS(dispstruct.ticks));
      
    }
}