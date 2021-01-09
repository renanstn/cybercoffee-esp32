#include <PubSubClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
/**
   A lib ArduinoJson deve estar na versão 5
   A lib Ardafruit SSD1306 deve ser instalada
   A lib Ardafruit GFX Library deve ser instalada
*/

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

const int serialSpeed = 115200;
const char* ssid = "ssid";
const char* password = "password";
const char* mqttServer = "mqtt.beebotte.com";
const int mqttPort = 1883;
const char* mqttToken = "token:<token>";
const char* mqttChannel = "esp32";
const char* mqttResource = "notification";

int  x, minX;
String notification;

WiFiClient espClient;
PubSubClient client(espClient);
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

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
  Serial.print("data: ");
  Serial.print(data);
  Serial.println();
  setScrollingMessage(data);
}

void mqttPublish(const char* resource, char* data, bool persist) {
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
  sprintf(topic, "%s/%s", mqttChannel, resource);
  // Now publish the char buffer to Beebotte
  client.publish(topic, buffer);
  Serial.print("Message published in: ");
  Serial.println(topic);
  Serial.print("Content:");
  Serial.println(buffer);
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
  // Subscribe to notifications channel
  char topic[64];
  sprintf(topic, "%s/%s", mqttChannel, mqttResource);
  client.subscribe(topic);
  Serial.println("Setup MQTT finished");
}

void setupDisplay() {
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setTextWrap(false);
  x = display.width();
}

void setScrollingMessage(String message) {
  minX = -12 * message.length();
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(message);
  display.display();
  notification = message;
}

void scrollMessage() {
  display.clearDisplay();
  display.setCursor(x, 0);
  display.print(notification);
  display.display();
  if (--x < minX) x = display.width();
}

void setup() {
  Serial.begin(serialSpeed);
  setupWifi();
  setupMQTT();
  setupDisplay();
  mqttPublish(mqttResource, "ESP32 initialized", false);
}

void loop() {
  client.loop();
  scrollMessage();
}
