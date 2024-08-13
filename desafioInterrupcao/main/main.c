#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <ultrasonic.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <esp_log.h>
#include <driver/gpio.h>

#define MAX_DISTANCE_CM 500 // 5m max
#define TRIGGER_GPIO 18
#define ECHO_GPIO 5

#define MIN_DISTANCE_CM 0.01


#define BT_IO 0

QueueHandle_t queueLeitura;
SemaphoreHandle_t sem_sinc;


void vTaskLeitura(void* pvParameters);
void vTaskAlarme(void* pvParameters);
void vTaskLimiar(void* pvParameters);

ultrasonic_sensor_t sensor;
float limiar;

static void IRAM_ATTR gpio_isr_handler(void* args)
{
    BaseType_t HPTW = pdFALSE;

    xSemaphoreGiveFromISR(sem_sinc,&HPTW);

    if(HPTW == pdTRUE)
    {
        portYIELD_FROM_ISR();
    }
}

void app_main(void)
{
    // Configuração Periféricos
    gpio_reset_pin(BT_IO);
    gpio_set_direction(BT_IO,GPIO_MODE_INPUT);

    sensor.trigger_pin = TRIGGER_GPIO;
    sensor.echo_pin = ECHO_GPIO;

    gpio_set_intr_type(BT_IO,GPIO_INTR_NEGEDGE);
    gpio_intr_enable(BT_IO);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(BT_IO, gpio_isr_handler, NULL);

    ultrasonic_init(&sensor);

    limiar = 0.1; // LIMIAR INICIAL EM CM


    queueLeitura= xQueueCreate( 100, sizeof(float));

    sem_sinc = xSemaphoreCreateBinary();
    
    xTaskCreate(vTaskLeitura, "Task Leitura", 2048, NULL, 1, NULL);
    xTaskCreate(vTaskAlarme, "Task Alarme", 2048, NULL, 1, NULL);
    xTaskCreate(vTaskLimiar, "Task Limiar", 2048, NULL, 2, NULL);


}
void vTaskLeitura(void* pvParameters){
    
    ESP_LOGI("LEITURA","Task Leitura inicializando");
    float distance;

    while (1)
    {
        ultrasonic_measure(&sensor, MAX_DISTANCE_CM, &distance);
        xQueueSendToBack(queueLeitura, &distance, portMAX_DELAY);
        ESP_LOGI("LEITURA","Dado  enviado: %.3f", distance);
        vTaskDelay(pdMS_TO_TICKS(200));

    }
    
}

void vTaskAlarme(void* pvParameters){
    
    ESP_LOGI("ALARME","Task Alarme inicializando");
    float distanciaMed;
    while (1)
    {
        xQueueReceive(queueLeitura, &distanciaMed, portMAX_DELAY);
        if(distanciaMed < limiar){
            ESP_LOGI("ALARME", "Distância medida menor que limiar, ALARME!!!");
        }
    }
    
}
void vTaskLimiar(void* pvParameters){
    ESP_LOGI("LIMIAR","Task Limiar inicializando");
    while (1)
    {
        xSemaphoreTake(sem_sinc, portMAX_DELAY);

        ESP_LOGI("AJUSTE", "Limiar anterior: %.3f", limiar);
        limiar -= 0.01;
        if(limiar <= MIN_DISTANCE_CM){
            limiar = 0.1;
        }
        ESP_LOGI("AJUSTE", "Limiar ajustado: %.3f", limiar);
    }
    
}