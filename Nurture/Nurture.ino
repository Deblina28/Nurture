#include "DHTStable.h"
#include <Wire.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define DHT11_PIN 27
#define mq2 32
#define soilm 33
#define bulb 13
#define pump 12

DHTStable DHT;
WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

TaskHandle_t task1;
TaskHandle_t task2;
TaskHandle_t task3;

const char *ssid = "Dropkick";
const char *pswd = "avijitdasxp";
char *mqttServer = "saas.theakiro.com";
int mqttPort = 1883;


double temp = 0.0, humid = 0.0, soilraw = 0.0, mqraw = 0.0, setTemp = 0.0, setHumid = 0.0, setMoist = 0.0;
String t = "", rcv = "";
boolean seamaphore = true;
char buf[40];
long ls = 0;

void setup() {
  Serial.begin(115200);

  connectToWiFi();
  setupMQTT();
  if (!mqttClient.connected())
    connectMQTT();
  mqttClient.subscribe("Room");

  xTaskCreatePinnedToCore(readSensors, "Task1", 50000, NULL, 1, &task1, 0);
  xTaskCreatePinnedToCore(handleMQTT, "Task2", 50000, NULL, 2, &task2, 1);

}

void loop()
{

}

void connectToWiFi()
{
  Serial.print("Connecting to ");

  WiFi.begin(ssid, pswd);
  Serial.println(ssid);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(500);
  }
  Serial.println("Connected to WiFi.");
}

void setupMQTT()
{
  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setCallback(callback);
}

void connectMQTT()
{
  Serial.println("Connecting to Akiro...");
  while (!mqttClient.connected())
  {
    String clientId = "sicpi";
    if (mqttClient.connect(clientId.c_str(), "qWdhxyJzxnZY:@sicpi", "sicpi123" ))
      Serial.println("Connected to Akiro.");
  }
}

void callback(char* topic, byte* payload, unsigned int length)
{
  rcv = "";

  for (int i = 0; i < length; i++)
    rcv = rcv + (char)payload[i];

  rcv.toLowerCase();

  handleAct();

  //Serial.println(rcv);
}

void handleMQTT(void *pv2)
{
  for (;;)
  {

    if ((millis() - ls > 10000) && seamaphore)
    {
      t = String(temp) + " " + String(humid) + " " + String(soilraw) + " " + String(mqraw);
      t.toCharArray(buf, 30);
      mqttClient.publish("sicpi", buf);
      ls = millis();
    }
    if (!mqttClient.connected())
      connectMQTT();
    mqttClient.loop();
  }
}


void handleAct()
{
  rcv.toLowerCase();
  String t = "";
  if (rcv.startsWith("set"))
  {
    if (rcv.charAt(4) == 't')
    {
      setTemp = rcv.substring(rcv.indexOf('=') + 2).toFloat();
      t = "Setting Temperature to : " + setTemp;
      t.toCharArray(buf, 35);
      mqttClient.publish("sicpi", buf);
    }

    else if (rcv.charAt(4) == 'm')
    {
      setMoist = rcv.substring(rcv.indexOf('=') + 2).toFloat();
      t = "Setting Moisture to : " + setMoist;
      t.toCharArray(buf, 35);
      mqttClient.publish("sicpi", buf);
    }

    else
    {
      t = "Wrong Syntax";
      t.toCharArray(buf, 35);
      mqttClient.publish("sicpi", buf);
    }
  }

  else if (rcv.startsWith("stop"))
  {
    t = "Publishing Stopped";
    t.toCharArray(buf, 35);
    mqttClient.publish("sicpi", buf);
    seamaphore = false;
  }

  else if (rcv.startsWith("start"))
  {
    seamaphore = true;
    t = "Publishing Started";
    t.toCharArray(buf, 35);
    mqttClient.publish("sicpi", buf);
  }


  Serial.println(rcv);
  Serial.print(setHumid);
  Serial.print(" ");
  Serial.print(setTemp);
  Serial.print(" ");
  Serial.println(setMoist);

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
