#include <Arduino.h>
//API REF: https://www.freertos.org/a00125.html
/* Bibliotecas FreeRTOS */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
/* Definição de Pinos*/
#define LED 2

void vTaskBlink(void *pvParameters)
{
  pinMode(LED, OUTPUT);
  while (1)
  {
    digitalWrite(LED, !digitalRead(LED));
    vTaskDelay(pdMS_TO_TICKS(200));
  }
}
void vTaskCounter(void *pvParameters)
{
  Serial.begin(9600);
  int count = 0;
  while (1)
  {
    Serial.println("Counter " + String(count++));
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void setup()
{

  xTaskCreate(vTaskBlink, "TASK_BLINK", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
  xTaskCreate(vTaskCounter, "TASK_COUNTER", configMINIMAL_STACK_SIZE + 1024, NULL, 2, NULL);
}

void loop()
{
  //remover a task loop da aplicação para libertar tempo de CPU
  vTaskDelete(NULL);
}
