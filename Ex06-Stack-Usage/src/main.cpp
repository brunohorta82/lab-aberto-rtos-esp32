#include <Arduino.h>
//API REF: https://www.freertos.org/uxTaskGetStackHighWaterMark.html
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
  UBaseType_t stackUsage;
  int pin = (int)pvParameters;
  pinMode(pin, OUTPUT);
  while (1)
  {
    digitalWrite(pin, !digitalRead(pin));
    vTaskDelay(pdMS_TO_TICKS(200));

    stackUsage = uxTaskGetStackHighWaterMark(NULL);
    Serial.print(pcTaskGetTaskName(NULL));
    Serial.print(" : ");
    Serial.println(stackUsage);
  }
}
void vTaskCounter(void *pvParameters)
{
  UBaseType_t stackUsage;
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
    stackUsage = uxTaskGetStackHighWaterMark(NULL);
    Serial.print(pcTaskGetTaskName(NULL));
    Serial.print(" : ");
    Serial.println(stackUsage);
  }
}

void setup()
{
  /**
   * ESP32 tem 2 Nucleos
   * 0 -> PRO_CPU_NUM
   * 1 -> APP_CPU_NUM
   **/
  xTaskCreatePinnedToCore(vTaskAtuator, "TASK_LED", configMINIMAL_STACK_SIZE, (void *)LED, 1, &taskLedHandle, PRO_CPU_NUM);
  xTaskCreatePinnedToCore(vTaskAtuator, "TASK_RELAY", configMINIMAL_STACK_SIZE, (void *)RELAY, 3, &taskRelayHandle, PRO_CPU_NUM);
  xTaskCreatePinnedToCore(vTaskCounter, "TASK_COUNTER", configMINIMAL_STACK_SIZE + 1024, NULL, 2, &taskCounterHandle, APP_CPU_NUM);
}

void loop()
{
  //remover a task loop da aplicação para libertar tempo de CPU
  vTaskDelete(NULL);
}
