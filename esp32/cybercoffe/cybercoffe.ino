#include <PubSubClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>

const char* ssid = "wifi-id";
const char* password = "wifi-senha";
const char* mqttServer = "mqtt.beebotte.com";
const int mqttPort = 1883;
const char* mqttToken = "token:XXXXXXXXXX";
const char* mqttChannel = "esp32";
const char* mqttResource = "notification";

WiFiClient espClient;
PubSubClient client(espClient);

void setupWifi() {
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("Connecting to WiFi:");
    Serial.println(ssid);
  }

  Serial.println("Connected to the WiFi network");
  Serial.println();
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setupMQTT() {
  client.setServer(mqttServer, mqttPort);
  client.setCallback(mqttCallback);
}

void setup() {
  Serial.begin(115200);
  setupWifi();
  setupMQTT();
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Decode the JSON payload
  StaticJsonDocument<200> doc;
  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, payload);

  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  char* data = doc["data"];

  // Print the received value to serial monitor for debugging
  Serial.print("Received message of length ");
  Serial.print(length);
  Serial.println();
  Serial.print("data ");
  Serial.print(data);
  Serial.println();
}

void loop() {
  client.loop();
}
