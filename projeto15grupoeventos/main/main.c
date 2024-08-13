#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/event_groups.h>
#include <driver/gpio.h>
#include <esp_log.h>

#define BT_IO 0

#define EV_VAL (1 << 0)
#define EV_BT (1 << 1)

EventGroupHandle_t ev_group;

void vTask1(void *pvparameters);
void vTask2(void *pvparameters);
void vTask3(void *pvparameters);

void app_main(void)
{
    gpio_reset_pin(BT_IO);
    gpio_set_direction(BT_IO, GPIO_MODE_INPUT);

    ev_group = xEventGroupCreate();

    xTaskCreate(vTask1, "Task 1", 2048, NULL, 1, NULL);
    xTaskCreate(vTask2, "Task 2", 2048, NULL, 1, NULL);
    xTaskCreate(vTask3, "Task 3", 2048, NULL, 1, NULL);
}

void vTask1(void *pvparameters)
{

    while (true)
    {
        if (gpio_get_level(BT_IO) == 0)
        {
            ESP_LOGW("TASK1", "Botão pressionado!");
            xEventGroupSetBits(ev_group, EV_BT);
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
    
}

void vTask2(void *pvparameters)
{
    uint16_t valor = 0;
    
    while (true)
    {
        valor += rand() % 10;
        ESP_LOGI("TASK2", "Valor = %d.", valor);
        if (valor > 100)
        {
            valor = 0;
            xEventGroupSetBits(ev_group, EV_VAL);
        }
        
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
    
}
void vTask3(void *pvparameters)
{
    EventBits_t bits;
    
    while (true)
    {
        bits = xEventGroupWaitBits(ev_group, (EV_VAL | EV_BT), pdTRUE, pdTRUE, portMAX_DELAY);
        if (bits == EV_BT)
        {
            ESP_LOGI("TASK3", "Detectado EV_BT");
        }
        if (bits == EV_VAL)
        {
            ESP_LOGI("TASK3", "Detectado EV_VAL");
        }
        if (bits == (EV_BT | EV_VAL))
        {
            ESP_LOGI("TASK3", "Ocorreram os dois Eventos!");
        }
        
    }

}

