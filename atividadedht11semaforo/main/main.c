#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <esp_log.h>
#include <dht11.h>
#include <driver/gpio.h>

#define GPIO_BT 0

void taskBT(void* pvParameters);
void taskSensor(void* pvParameters);
void taskDisplay(void* pvParameters);

// struct dht11_reading
// {
//     int status;
//     int temperature;
//     int humidity;
// };

struct dht11_reading data;

SemaphoreHandle_t sem_mutex;
SemaphoreHandle_t sem_sinc;
SemaphoreHandle_t sem_display;

void app_main(void)
{
    gpio_reset_pin(GPIO_BT);
    gpio_set_direction(GPIO_BT, GPIO_MODE_INPUT);

    DHT11_init(DHT11_PIN);

    sem_mutex = xSemaphoreCreateMutex();
    sem_sinc = xSemaphoreCreateBinary();
    sem_display = xSemaphoreCreateBinary();

    xTaskCreatePinnedToCore(taskBT, "Botao", 2048, NULL, 2, NULL, 0);
    xTaskCreatePinnedToCore(taskSensor, "Sensor", 2048, NULL, 3, NULL, 0);
    xTaskCreatePinnedToCore(taskDisplay, "Display", 2048, NULL, 2, NULL, 0);

}

void taskBT(void* pvParameters)
{

    ESP_LOGI("BT","Task Botão inicializando");
    while (1)
    {
        if(gpio_get_level(GPIO_BT) == 0){
            ESP_LOGI("BT","Botão pressionado");
            xSemaphoreGive(sem_sinc);
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }  
}


void taskSensor(void* pvParameters)
{
    BaseType_t status;

    ESP_LOGI("Sensor","Task Sensor inicializando");
    while(1)
    {
        status = xSemaphoreTake(sem_sinc, portMAX_DELAY);
        if(status == pdTRUE)
        {
            xSemaphoreTake(sem_mutex, portMAX_DELAY);
            data = DHT11_read();
            xSemaphoreGive(sem_mutex);
            xSemaphoreGive(sem_display);

        }
    }
}

void taskDisplay(void* pvParameters)
{
    ESP_LOGI("Display","Task Display inicializando");
    while(1)
    {
        xSemaphoreTake(sem_display, portMAX_DELAY);
        xSemaphoreTake(sem_mutex, portMAX_DELAY);
        ESP_LOGI("Display", "Temperatura: %d , Umidade: %d", data.temperature, data.humidity);
        xSemaphoreGive(sem_mutex);
    }
    
}

