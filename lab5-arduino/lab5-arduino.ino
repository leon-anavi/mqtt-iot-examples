#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "Adafruit_HTU21DF.h"

Adafruit_HTU21DF htu = Adafruit_HTU21DF();

float sensorTemperature = 0;
float sensorHumidity = 0;

unsigned long sensorPreviousMillis = 0;
const long sensorInterval = 5000;

//WiFi
const char* ssid = "test";
const char* password = "password";
const char* mqtt_server = "iot.eclipse.org";

//MQTT
WiFiClient espClient;
PubSubClient mqttClient(espClient);

void connectWiFi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("connected to MQTT broker!");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  
  connectWiFi();
  mqttClient.setServer(mqtt_server, 1883);

  htu.begin();
}

void publishSensorData(const char* subTopic, const char* key, const float value)
{
  StaticJsonBuffer<100> jsonBuffer;
  char payload[100];
  JsonObject& json = jsonBuffer.createObject();
  json[key] = value;
  json.printTo((char*)payload, json.measureLength() + 1);
  char topic[200];
  sprintf(topic,"home/room/%s", subTopic);
  mqttClient.publish(topic, payload, true);
}

void loop() {
  if (!mqttClient.connected()) {
    reconnect();
  }
  mqttClient.loop();
  
  const unsigned long currentMillis = millis();
  if (sensorInterval <= (currentMillis - sensorPreviousMillis))
  {
    sensorPreviousMillis = currentMillis;
  
    // Check if temperature has changed
    const float tempTemperature = htu.readTemperature();
    if (1 <= abs(tempTemperature - sensorTemperature))
    {
      // Print new temprature value
      sensorTemperature = tempTemperature;
      Serial.print("Temperature: ");
      Serial.print(sensorTemperature);
      Serial.println("C");
      
      // Publish new temperature value through MQTT
      publishSensorData("temperature", "temperature", sensorTemperature);
    }
    
    // Check if humidity has changed
    const float tempHumidity = htu.readHumidity();
    if (1 <= abs(tempHumidity - sensorHumidity))
    {
        // Print new humidity value
        sensorHumidity = tempHumidity;
        Serial.print("Humidity: ");
        Serial.print(sensorHumidity);
        Serial.println("%");
    
        // Publish new humidity value through MQTT
        publishSensorData("humidity", "humidity", sensorHumidity);
    }
  }
}
