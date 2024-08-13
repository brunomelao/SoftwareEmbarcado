#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_log.h>
#include <driver/gpio.h>
#include <freertos/semphr.h>

#define BT_GPIO 0
#define LED_GPIO 2

void vTask_BT(void* pvParameters);
void vTask_LED(void* pvParameters);

SemaphoreHandle_t sem_sinc;

void app_main(void)
{
    gpio_reset_pin(BT_GPIO);
    gpio_set_direction(BT_GPIO, GPIO_MODE_INPUT);

    gpio_reset_pin(LED_GPIO);
    gpio_set_direction(LED_GPIO, GPIO_MODE_OUTPUT);

    sem_sinc = xSemaphoreCreateBinary();
    if(sem_sinc != NULL){
        xTaskCreatePinnedToCore(vTask_BT,"Task Botão", 2048, NULL, 1, NULL, 1);
        xTaskCreatePinnedToCore(vTask_LED,"Task Led", 2048, NULL, 1, NULL, 0);
    }
}

void vTask_BT(void* pvParameters)
{
    uint8_t cnt = 0;

    ESP_LOGI("BT","Task Botão inicializando");
    while (1)
    {
        if(gpio_get_level(BT_GPIO) == 0){
            ESP_LOGI("BT","Botão pressionado CNT= %d", ++cnt);
            xSemaphoreGive(sem_sinc);
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }  
}

void vTask_LED(void* pvParameters)
{
    uint8_t cnt = 0;  
    BaseType_t status;
    ESP_LOGI("LED","Task Led inicializando");
    while (1)
    {
        status = xSemaphoreTake(sem_sinc, portMAX_DELAY);
        if(status==pdTRUE)
        {
            ESP_LOGI("LED","Task Led executando");
            gpio_set_level(LED_GPIO, (cnt++) % 2);
        }
        else
        {
            ESP_LOGE("LED","Delay expirou");
        }
    }
}