#include <Arduino.h>
//API REF: https://www.freertos.org/a00125.html
/* Bibliotecas FreeRTOS */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
/* Definição de Pinos*/
#define LED 2
#define RELAY 4

/* Definição de Handlers */
TaskHandle_t taskLedHandle = NULL;
TaskHandle_t taskRelayHandle = NULL;
TaskHandle_t taskCounterHandle = NULL;

void vTaskAtuator(void *pvParameters)
{
  int pin = (int)pvParameters;
  pinMode(pin, OUTPUT);
  while (1)
  {
    digitalWrite(pin, !digitalRead(pin));
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
    if (count == 10)
    {
      Serial.println("Task Relay suspended");
      vTaskSuspend(taskRelayHandle); //MEMORIA OCUPADA NA STACK PELA TASK RELAY NÃO É LIBERTADA
    }

    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}

void setup()
{
  xTaskCreate(vTaskAtuator, "TASK_LED", configMINIMAL_STACK_SIZE, (void *)LED, 1, &taskLedHandle);
  xTaskCreate(vTaskAtuator, "TASK_RELAY", configMINIMAL_STACK_SIZE, (void *)RELAY, 3, &taskRelayHandle);
  xTaskCreate(vTaskCounter, "TASK_COUNTER", configMINIMAL_STACK_SIZE + 1024, NULL, 2, &taskCounterHandle);
}

void loop()
{
  //remover a task loop da aplicação para libertar tempo de CPU
  vTaskDelete(NULL);
}
