#include <stdio.h>
#include <string.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/timers.h>
#include <freertos/queue.h>
#include <freertos/semphr.h>
#include <esp_log.h>
#include <driver/gpio.h>

#define BT_IO 0

SemaphoreHandle_t sem_sinc, sem_bin;
TimerHandle_t timer1, timerfim;
QueueHandle_t fila;


char caractere;
char codigo[10];

char *characters[] = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M","N", "O", "P", "Q", "R", "S", "T", "U",
                      "V", "W", "X", "Y", "Z", "1", "2", "3", "4", "5", "6", "7", "8", "9", "0"};


char *morsecode[] = {".-","-...","-.-.","-..",".","..-.","--.","....","..",".---", "-.-",".-..","--","-.","---",".--.","--.-",
                     ".-.","...","-","..-", "...-",".--","-..-","-.--","--..", ".----","..---","...--","....-", ".....", "-....",
                     "--...","---..","----.","-----"};


static void IRAM_ATTR gpio_isr_handler(void* args)
{

    ESP_LOGE("BT", "Botao pressionado.");
    xTimerStartFromISR(timer1, 0);
    xTimerStartFromISR(timerfim, 0);
    
}

void timer_cb(TimerHandle_t xtimer){
    ESP_LOGE("TIMER1","Evento Timer 1.");
    uint16_t cnt;

    if(gpio_get_level(BT_IO) == 0)
    {
        caractere='-';
    }
    else
    {
        caractere= '.';
    }   

    cnt = (uint16_t)pvTimerGetTimerID(timer1);
    codigo[cnt]=caractere;

    cnt++;
    vTimerSetTimerID(timer1, (void *)cnt);
  

}

void timerfim_cb(TimerHandle_t xtimer){
    codigo[strlen(codigo)+1]='\0';
    xSemaphoreGive(sem_sinc);
}


void vTaskReconhece(void* pvparameters);

void app_main(void)
{
    gpio_reset_pin(BT_IO);
    gpio_set_direction(BT_IO,GPIO_MODE_INPUT);

    gpio_set_intr_type(BT_IO,GPIO_INTR_NEGEDGE);
    gpio_intr_enable(BT_IO);

    gpio_install_isr_service(0);
    gpio_isr_handler_add(BT_IO, gpio_isr_handler, NULL);

    timer1 = xTimerCreate("Timer1", pdMS_TO_TICKS(200), pdTRUE, (void *)0, timer_cb);
    timerfim = xTimerCreate("Timer1", pdMS_TO_TICKS(3000), pdTRUE, (void *)0, timer_cb);
    sem_sinc = xSemaphoreCreateBinary();

    xTaskCreate(vTaskReconhece, "Task Reconhece caractere", 2048, NULL,1,NULL);
}
void vTaskReconhece(void* pvparameters)
{
    int cnt=0;
    char letra=' ';
    ESP_LOGI("RECONHECE","Task Reconhece inicializada");
    while(1)
    {
        xSemaphoreTake(sem_sinc, portMAX_DELAY);

        for(int i=0; codigo[i]!='\0'; i++)
        {
            for(int j=0; strlen(morsecode[i]); j++)
            {
                if(codigo[i]==morsecode[i][j])
                {
                    cnt++;
                }
            }
            if (cnt == strlen(codigo))
            {
                letra = characters[i];
                break;
            }
        }
        ESP_LOGI("RECONHECE", "Código decodificado: %c",letra);

        vTaskDelay(pdMS_TO_TICKS(200));
       
    }
}