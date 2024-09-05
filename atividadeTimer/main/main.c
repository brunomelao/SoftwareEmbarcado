#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/timers.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <esp_log.h>
#include <driver/gpio.h>

#define BT_GPIO 0

SemaphoreHandle_t sem_sinc, sem_bin;
TimerHandle_t timer1, timerfim;

char caractere;
char codigo[6]="";
uint16_t cnt = 0;

char *characters[] = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M","N", "O", "P", "Q", "R", "S", "T", "U",
                      "V", "W", "X", "Y", "Z", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0"};


char *morsecode[] = {".-","-...","-.-.","-..",".","..-.","--.","....","..",".---", "-.-",".-..","--","-.","---",".--.","--.-",
                     ".-.","...","-","..-", "...-",".--","-..-","-.--","--..", ".----","..---","...--","....-", ".....", "-....",
                     "--...","---..","----.","-----"};


static void IRAM_ATTR gpio_isr_handler(void* args)
{
    xTimerStartFromISR(timer1, NULL);
    
}

void timer_cb(TimerHandle_t xtimer){
    ESP_LOGE("TIMER1","Evento Timer 1.");
    

    if(gpio_get_level(BT_GPIO) == 0)
    {
        caractere='-';
        ESP_LOGI("TIMER","Traço");
    }
    else
    {
        caractere= '.';
        ESP_LOGI("TIMER","Ponto");
    }   
    codigo[cnt]=caractere;

    cnt++; 
    if (cnt==6){
        memset(codigo,0,strlen(codigo));
        cnt=0;
    }
    xTimerStart(timerfim, 0);

}

void timerfim_cb(TimerHandle_t xtimer){
    codigo[cnt]='\0';
    ESP_LOGI("TIMERFIM","FIM");

    xSemaphoreGive(sem_sinc);
}


void vTaskReconhece(void* pvparameters);

void app_main(void)
{
    gpio_reset_pin(BT_GPIO);
    gpio_set_direction(BT_GPIO,GPIO_MODE_INPUT);

    gpio_set_intr_type(BT_GPIO,GPIO_INTR_NEGEDGE);
    gpio_intr_enable(BT_GPIO);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(BT_GPIO, gpio_isr_handler, NULL);

    timer1 = xTimerCreate("Timer1", pdMS_TO_TICKS(250), pdFALSE, (void *)0, timer_cb);
    timerfim = xTimerCreate("TimerFim", pdMS_TO_TICKS(3000), pdFALSE, (void *)0, timerfim_cb);
    sem_sinc = xSemaphoreCreateBinary();

    xSemaphoreTake(sem_sinc,0);

    xTaskCreate(vTaskReconhece, "Task Reconhece caractere", 4096, NULL,2,NULL);
}
void vTaskReconhece(void* pvparameters)
{
    int cont=0;
    char letra="";
    ESP_LOGI("RECONHECE","Task Reconhece inicializada");
    while(1)
    {
        xSemaphoreTake(sem_sinc, portMAX_DELAY);

        for (uint16_t j = 0; j < 36; j++)
        {
            if(strcmp(codigo, morsecode[j]) == 0)
            {
                ESP_LOGE("Display", "Caractere identificado: %s", characters[j]);
                break;
            }
        }
        memset(codigo,0,strlen(codigo));
        cnt=0;

        vTaskDelay(pdMS_TO_TICKS(300));
       
    }
}