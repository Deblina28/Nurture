#include "DHTStable.h"
#include <Wire.h>


#define DHT11_PIN 27
#define mq2 32
#define soilm 33
#define bulb 13
#define pump 12

DHTStable DHT;
TaskHandle_t task1;


double temp = 0.0, humid = 0.0, soilraw = 0.0, mqraw = 0.0;
long ls=0;

void setup() {
  Serial.begin(115200);


xTaskCreatePinnedToCore(readSensors, "Task1", 20000, NULL, 2, &task1, 1);


}

void loop()
{

}

void printSensors()
{
    Serial.print(humid);
    Serial.print(" ");
    Serial.print(temp);
    Serial.print(" ");
    Serial.print(mqraw);
    Serial.print(" ");
    Serial.println(soilraw);
}

void readSensors(void *pv)
{
  for (;;)
  {
    if (!DHT.read11(DHT11_PIN))
    {
      temp = DHT.getTemperature();
      humid = DHT.getHumidity();
    }

    mqraw = analogRead(mq2);
    soilraw = analogRead(soilm);
    printSensors();
    vTaskDelay(500 / portTICK_PERIOD_MS);
  
  }
}
