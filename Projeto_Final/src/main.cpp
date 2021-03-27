#include <Arduino.h>
/* Bibliotecas FreeRTOS */
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
/* Biblioteca Wifi */
#include <WiFi.h>
/* Biblioteca MQTT exemplos em : https://platformio.org/lib/show/89/PubSubClient/examples */
#include <PubSubClient.h>
/* Biblioteca Sensor */
#include <Adafruit_Sensor.h>
#include <DHT.h>
/* Biblioteca Servo exemplos em : https://platformio.org/lib/show/4744/ESP32Servo/examples */
#include <ESP32Servo.h>

/* Definição de Handlers de Fila para publicar leituras */
QueueHandle_t xQueue;

//Estrutura para guardar valores do sensor
struct MySensor
{
  float temperature;
  float humidity;
};
void vTaskServo(void *pvParameters)
{
  Servo myservo; // create servo object to control a servo
  // twelve servo objects can be created on most boards
  int pos = 0;        // variable to store the servo position
  myservo.attach(13); // attaches the servo on pin 9 to the servo object
  while (1)
  {
    for (pos = 0; pos <= 180; pos += 1)
    { // goes from 0 degrees to 180 degrees
      // in steps of 1 degree
      myservo.write(pos);            // tell servo to go to position in variable 'pos'
      vTaskDelay(pdMS_TO_TICKS(15)); // waits 15ms for the servo to reach the position
    }
    for (pos = 180; pos >= 0; pos -= 1)
    {                                // goes from 180 degrees to 0 degrees
      myservo.write(pos);            // tell servo to go to position in variable 'pos'
      vTaskDelay(pdMS_TO_TICKS(15)); // waits 15ms for the servo to reach the position
    }
  }
}
void vTaskPublish(void *pvParameters)
{
  Serial.begin(9600);
  // Configuração da tua Rede Wi-Fi
  const char *ssid = "IOTBH";
  const char *password = "IOT2017@";
  // Ligar ao Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    vTaskDelay(pdMS_TO_TICKS(1000));
    Serial.println("Connecting to WiFi..");
  }
  WiFiClient espClient;                         //Acesso a Interface WiFi
  PubSubClient mqttClient(espClient);           //Criação do cliente MQTT
  mqttClient.setServer("broker.emqx.io", 1883); // COnfiguração do Servidor
  MySensor sensor;
  while (1)
  {
    if (xQueueReceive(xQueue, &sensor, portMAX_DELAY) == pdTRUE) //API REF: https://www.freertos.org/a00118.html
    {
      if (!mqttClient.connected())
      {
        mqttClient.connect(String(WiFi.macAddress()).c_str());
        Serial.printf("OFFLINE: Temperatura: %f Humidade: %f \n", sensor.temperature, sensor.humidity);
      }
      else
      {
        Serial.printf("ONLINE: Temperatura: %f Humidade: %f \n", sensor.temperature, sensor.humidity);
        String message = "";
        message.concat("Bruno Horta ");
        message.concat("Temperature: ");
        message.concat(sensor.temperature);
        message.concat(" ");
        message.concat("Humidity: ");
        message.concat(sensor.humidity);
        mqttClient.publish("sensor/bruno_horta/status", message.c_str());
      }
    }
    vTaskDelay(pdMS_TO_TICKS(1)); //Prevenir Starvation
  }
}
void vTaskSensor(void *pvParameters)
{ // Sensor DHT
  DHT dht(5, DHT11);
  //Iniciar o sensor de temperatura
  dht.begin();
  MySensor sensor;
  while (1)
  {
    sensor.temperature = dht.readTemperature();
    sensor.humidity = dht.readHumidity();
    vTaskDelay(pdMS_TO_TICKS(2000));
    if (!isnan(sensor.temperature) && !isnan(sensor.humidity))
    {
      xQueueSend(xQueue, &sensor, portMAX_DELAY); //API REF: https://www.freertos.org/a00117.html
    }
  }
}
void setup()
{
  // Inicialização da Fila
  xQueue = xQueueCreate(5, sizeof(MySensor)); //API REF: https://www.freertos.org/a00116.html
  /**
   * ESP32 tem 2 Nucleos
   * 0 -> PRO_CPU_NUM
   * 1 -> APP_CPU_NUM
   **/
  xTaskCreatePinnedToCore(vTaskPublish, "TASK_PUBLISH", configMINIMAL_STACK_SIZE + 2024, NULL, 1, NULL, PRO_CPU_NUM);
  xTaskCreatePinnedToCore(vTaskSensor, "TASK_SENSOR", configMINIMAL_STACK_SIZE + 1024, NULL, 3, NULL, APP_CPU_NUM);
  xTaskCreatePinnedToCore(vTaskServo, "TASK_SERVO", configMINIMAL_STACK_SIZE + 1024, NULL, 2, NULL, APP_CPU_NUM);
}

void loop()
{
  //remover a task loop da aplicação para libertar tempo de CPU
  vTaskDelete(NULL);
}