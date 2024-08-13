#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/message_buffer.h>
#include <driver/gpio.h>
#include <esp_log.h>

#define BT_IO 0

MessageBufferHandle_t buffer;

void vTask1(void* pvParameters);
void vTask2(void* pvParameters);

void app_main(void)
{
    gpio_reset_pin(BT_IO);
    gpio_set_direction(BT_IO, GPIO_MODE_INPUT);

    buffer = xStreamBufferCreate(25, 5);

    xTaskCreate(vTask1, "Task 1", 2048, NULL, 2, NULL);
    xTaskCreate(vTask2, "Task 2", 2048, NULL, 1, NULL);
}

void vTask1(void* pvParameters)
{   
    int cnt = 0;
    size_t bytes_env;
    while(1)
    {
        if(gpio_get_level(BT_IO) == 0)
        {
            cnt++;
            bytes_env=xStreamBufferSend(buffer, &cnt, sizeof(cnt), portMAX_DELAY);
            ESP_LOGI("TASK1", "%d bytes enviados. CNT = %d", bytes_env, cnt);
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}

void vTask2(void* pvParameters)
{
    uint8_t data;
    size_t bytes_rec;
    while(1)
    {
        bytes_rec = xStreamBufferReceive(buffer, &data, sizeof(data), portMAX_DELAY);
        ESP_LOGI("TASK2","%d bytes recebidos, DATA = %d", bytes_rec, data);
    }
}
