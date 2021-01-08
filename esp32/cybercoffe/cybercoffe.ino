#include <PubSubClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>

const char* ssid = "wifi_ssid";
const char* password = "wifi_password";
const char* mqttServer = "mqtt.beebotte.com";
const int mqttPort = 1883;
const char* mqttToken = "token";
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

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // decode the JSON payload
  StaticJsonBuffer<128> jsonInBuffer;
  JsonObject& root = jsonInBuffer.parseObject(payload);
  // Test if parsing succeeds.
  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }
  String data = root["data"];
  Serial.print("Received message of length ");
  Serial.print(length);
  Serial.println();
  Serial.print("data ");
  Serial.print(data);
  Serial.println();
}

void publish(const char* resource, char* data, bool persist) {
    StaticJsonBuffer<128> jsonOutBuffer;
    JsonObject& root = jsonOutBuffer.createObject();
    root["channel"] = mqttChannel;
    root["resource"] = resource;
    if (persist) {
        root["write"] = true;
    }
    root["data"] = data;
    // Now print the JSON into a char buffer
    char buffer[128];
    root.printTo(buffer, sizeof(buffer));
    // Create the topic to publish to
    char topic[64];
    // Now publish the char buffer to Beebotte
    client.publish(topic, buffer);
}

void setupMQTT() {
  client.setServer(mqttServer, mqttPort);
  client.setCallback(mqttCallback);
  while (!client.connected()) {
    Serial.println("Connecting to MQTT...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), mqttToken, NULL)) {
      Serial.println("connected");
    } else {
      Serial.print("failed with state ");
      Serial.println(client.state());
      delay(2000);
    }
  }
  publish(mqttResource, "Hello from ESP32", false);
  char topic[64];
  sprintf(topic, "%s/%s", mqttChannel, mqttResource);
  client.subscribe(topic);
  Serial.println("Setup MQTT finished");
}

void setup() {
  Serial.begin(115200);
  setupWifi();
  setupMQTT();
}

void loop() {
  client.loop();
}
