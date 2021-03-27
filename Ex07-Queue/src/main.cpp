#include <Arduino.h>
//API REF: https://www.freertos.org/a00018.html
/* Bibliotecas FreeRTOS */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
/* Definição de Pinos*/
#define LED 2
#define RELAY 4
/* Definição de Handlers de Fila */
QueueHandle_t xQueue;

/* Definição de Handlers */
TaskHandle_t taskCounterHandle = NULL;

void vTaskAtuator(void *pvParameters)
{

  int value = 0;
  pinMode(LED, OUTPUT);
  pinMode(RELAY, OUTPUT);
  while (1)
  {
    if (xQueueReceive(xQueue, &value, pdMS_TO_TICKS(1000)) == pdTRUE) //API REF: https://www.freertos.org/a00118.html
    {
      if (value % 2 == 0)
      {
        Serial.printf("NUMERO PAR [%d]\n", value);
        digitalWrite(RELAY, HIGH); //RELE É INVERTIDO
        digitalWrite(LED, HIGH);
      }
      else
      {
        Serial.printf("NUMERO IMPAR [%d]\n", value);
        digitalWrite(RELAY, LOW);
        digitalWrite(LED, LOW);
      }
    }
    else
    {
      Serial.println("TIME OUT");
    }

    vTaskDelay(pdMS_TO_TICKS(200));
  }
}
void vTaskCounter(void *pvParameters)
{
  Serial.begin(9600);
  int count = 0;
  while (1)
  {
    count++;
    xQueueSend(xQueue, &count, portMAX_DELAY); //API REF: https://www.freertos.org/a00117.html
    vTaskDelay(pdMS_TO_TICKS(500));
  }
}

void setup()
{

  // Inicialização da Fila
  xQueue = xQueueCreate(5, sizeof(int)); //API REF: https://www.freertos.org/a00116.html
  /**
   * ESP32 tem 2 Nucleos
   * 0 -> PRO_CPU_NUM
   * 1 -> APP_CPU_NUM
   **/
  xTaskCreatePinnedToCore(vTaskAtuator, "TASK_ATUATOR", configMINIMAL_STACK_SIZE + 1024, NULL, 1, NULL, PRO_CPU_NUM);
  xTaskCreatePinnedToCore(vTaskCounter, "TASK_COUNTER", configMINIMAL_STACK_SIZE + 1024, NULL, 2, &taskCounterHandle, APP_CPU_NUM);
}

void loop()
{
  //remover a task loop da aplicação para libertar tempo de CPU
  vTaskDelete(NULL);
}
