#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/timers.h>
#include <esp_log.h>
#include <driver/gpio.h>

#define BT_IO 0
TimerHandle_t timer1, timer2, timer3;

void vTaskBT(void* pvParameters);
void timer_cb(TimerHandle_t xtimer)
{
    uint16_t cnt;
    if(xtimer==timer1)
    {
        cnt = (uint16_t)pvTimerGetTimerID(timer1);
        cnt++;
        ESP_LOGI("TIMER1", "Evento do Timer 1. CNT = %d", cnt);
        vTimerSetTimerID(timer1, (void *)cnt);
    }
    else
    {
        cnt = (uint16_t)pvTimerGetTimerID(timer2);
        cnt++;
        ESP_LOGW("TIMER2", "Evento do Timer 2. CNT = %d", cnt);
        vTimerSetTimerID(timer2, (void *)cnt);
    }
}

void timer3_cb(TimerHandle_t xtimer){
    ESP_LOGE("TIMER3","Evento Timer 3.");
}

void app_main(void)
{
    gpio_reset_pin(BT_IO);
    gpio_set_direction(BT_IO, GPIO_MODE_INPUT);

    timer1 = xTimerCreate("Timer1", pdMS_TO_TICKS(1000), pdTRUE, (void *)0, timer_cb);

    timer2 = xTimerCreate("Timer2", pdMS_TO_TICKS(2000), pdTRUE, (void *)0, timer_cb);

    timer3 = xTimerCreate("Timer3", pdMS_TO_TICKS(5000), pdFALSE, NULL,timer3_cb);
    xTimerStart(timer1, 0);
    xTimerStart(timer2, 0);

    xTaskCreate(vTaskBT,"Task Botao", 2048, NULL,1,NULL );
}
void vTaskBT(void* pvParameters)
{
    while(1){
        if( gpio_get_level(BT_IO)==0){
            ESP_LOGE("BT", "Botao pressionado.");
            xTimerStart(timer3, 0);           
        }
        vTaskDelay(pdMS_TO_TICKS(200));
    }
}