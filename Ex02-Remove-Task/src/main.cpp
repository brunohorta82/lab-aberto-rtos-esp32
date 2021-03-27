#include <Arduino.h>
//API REF: https://www.freertos.org/a00126.html
/* Bibliotecas FreeRTOS */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
/* Definição de Pinos*/
#define LED 2

/* Definição de Handlers */
TaskHandle_t taskLedHandle = NULL;
TaskHandle_t taskCounterHandle = NULL;

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
    if (count >= 10)
    {
      if (taskLedHandle != NULL)
      {
        Serial.println("Task Led removed");
        vTaskDelete(taskLedHandle); //MEMORIA OCUPADA NA STACK PELA TASK LED LIBERTADA
        taskLedHandle = NULL;
      }
    }
    if (count >= 15)
    {
      Serial.println("Task Counter removed");
      vTaskDelete(NULL); //MEMORIA OCUPADA NA STACK PELA TASK COUNTER LIBERTADA
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void setup()
{

  xTaskCreate(vTaskBlink, "TASK_BLINK", configMINIMAL_STACK_SIZE, NULL, 1, &taskLedHandle); // Passagem do handle para a criação de Task
  xTaskCreate(vTaskCounter, "TASK_COUNTER", configMINIMAL_STACK_SIZE + 1024, NULL, 2, &taskCounterHandle);
}

void loop()
{
  //remover a task loop da aplicação para libertar tempo de CPU
  vTaskDelete(NULL);
}
