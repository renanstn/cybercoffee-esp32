#include <PubSubClient.h>
#include <WiFi.h>
#include <ArduinoJson.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "time.h"
/**
   A lib ArduinoJson deve estar na versão 5
   A lib Ardafruit SSD1306 deve ser instalada a parte na IDE
   A lib Ardafruit GFX Library deve ser instalada a parte na IDE
*/

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define BUZZER_PIN 15

const int channel = 0;
const int frequence = 2000;
const int resolution = 10;
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -3600 * 3;
const int daylightOffset_sec = 0;
const int serialSpeed = 115200;
const char* mqttServer = "mqtt.beebotte.com";
const int mqttPort = 1883;
const char* mqttChannel = "esp32";
const char* mqttResourceNotification = "notification";
const char* mqttResourcePomodoro = "pomodoro";
const char* ssid = "ssid";
const char* password = "password";
const char* mqttToken = "token:mqttToken";

String notification; // Armazenará a notificação atual sendo exibida
char hour[6]; // Armazenará a hora atual
int pomodoroTimer = 0; // Armazenará o timer do pomodoro, em segundos
int pomodoroMinutes;
int pomodoroSeconds;
char pomodoroClock[5]; // Armazena o timer do pomodoro formatado "99:99"
int  x, minX; // Usadas para movimentação horizontal de textos no display
TaskHandle_t pomodoroTimerHandle; // Handler do pomodoroTimer, que rodará em outro núcleo
eTaskState statusOf; // // Armazenará o status do pomodoroTimer, que rodará em outro núcleo

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

void setupBuzzer() {
  ledcSetup(channel, frequence, resolution);
  ledcAttachPin(BUZZER_PIN, channel);
}

void mqttCallback(char* topic, byte* payload, unsigned int length) {
  // Decode the JSON payload
  StaticJsonBuffer<128> jsonInBuffer;
  JsonObject& root = jsonInBuffer.parseObject(payload);
  // Test if parsing succeeds.
  if (!root.success()) {
    Serial.println("parseObject() failed");
    return;
  }
  String topic_string(topic);
  String data = root["data"];
  Serial.print("Received message in topic: ");
  Serial.print(topic);
  Serial.println();
  Serial.print("data: ");
  Serial.print(data);
  Serial.println();
  if (topic_string == "esp32/notification") {
    setScrollingMessage(data);
  } else if (topic_string == "esp32/pomodoro") {
    Serial.println("pomodoro");
    Serial.println(data);
    setPomodoro(data.toInt());
  }
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
  char topic[64];
  // Subscribe to notifications channel
  sprintf(topic, "%s/%s", mqttChannel, mqttResourceNotification);
  client.subscribe(topic);
  Serial.println("Subscribed to notifications channel");
  // Subscribe to pomodoro channel
  sprintf(topic, "%s/%s", mqttChannel, mqttResourcePomodoro);
  client.subscribe(topic);
  Serial.println("Subscribed to pomodoro channel");
  // Setup finished
  Serial.println("Setup MQTT finished");
}

void setupDisplay() {
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextWrap(false);
  x = display.width();
}

void setupClock() {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
}

void getClockInfo() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  strftime(hour, 6, "%H:%M", &timeinfo);
}

void secondsToMinuteSeconds(const int seconds, int &m, int &s) {
  int t = seconds;
  s = t % 60;
  t = (t - s) / 60;
  m = t % 60;
  t = (t - m) / 60;
}

void startPomodoroTimer(void *pvParameters) {
  while (pomodoroTimer > 0) {
    pomodoroTimer -= 1;
    secondsToMinuteSeconds(pomodoroTimer, pomodoroMinutes, pomodoroSeconds);
    delay(1000);
  }
  // Tocar o alarme ao fim do tempo
  //ledcWriteTone(channel, 2000);
  //delay(1000);
  //ledcWriteTone(channel, 0);
  pomodoroTimer = 0;
  vTaskDelete(NULL);
}

void showClock() {
  display.setTextSize(3);
  display.setCursor(20, 24);
  display.println(hour);
}

void showPomodoroTimer() {
  display.setTextSize(3);
  display.setCursor(20, 24);
  sprintf(pomodoroClock, "%02d:%02d", pomodoroMinutes, pomodoroSeconds);
  display.println(pomodoroClock);
}

void setScrollingMessage(String message) {
  minX = -12 * message.length();
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println(message);
  display.display();
  notification = message;
}

void setPomodoro(int time_in_seconds) {
  pomodoroTimer = time_in_seconds;
  xTaskCreatePinnedToCore(startPomodoroTimer, "startPomodoroTimer", 10000, NULL, 1, &pomodoroTimerHandle, 0);
}

void scrollMessage() {
  display.setTextSize(1);
  display.setCursor(x, 0);
  display.print(notification);
  if (--x < minX) x = display.width();
}

void setup() {
  Serial.begin(serialSpeed);
  setupWifi();
  //setupBuzzer();
  setupMQTT();
  setupDisplay();
  mqttPublish(mqttResourceNotification, "ESP32 initialized", false);
  setupClock();
  getClockInfo();
  showClock();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    setupWifi();
  }
  if (!client.connected()) {
    setupMQTT();
  }
  client.loop();
  display.clearDisplay();
  if (pomodoroTimer != 0) {
    // Modo pomodoro timer
    showPomodoroTimer();
    statusOf = eTaskGetState(pomodoroTimerHandle);
    // Verifica se a task pomodoro já terminou
    if (statusOf == eReady) {
      pomodoroTimer = 0;
    }
  } else {
    // Modo relógio
    getClockInfo();
    showClock();
  }
  scrollMessage();
  display.display();
}
