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
const int pomodoroWorkTime = 1500; // 25 minutes
const int pomodoroShortBreak = 300; // 5 minutes
const int pomodoroLongBreak = 600; // 10 minutes
const int pomodoroRounds = 4;
const char* ntpServer = "pool.ntp.org";
const long gmtOffset_sec = -3600 * 3;
const int daylightOffset_sec = 0;
const int serialSpeed = 115200;
const char* ssid = "ssid";
const char* password = "password";
const char* mqttServer = "mqtt.beebotte.com";
const int mqttPort = 1883;
const char* mqttToken = "token:token";
const char* mqttChannel = "esp32";
const char* mqttResource = "notification";

String notification; // Armazenará a notificação atual sendo exibida
char hour[6]; // Armazenará a hora atual
int pomodoroTimer = pomodoroWorkTime; // Armazenará o timer do pomodoro, em segundos
int pomodoroMinutes;
int pomodoroSeconds;
int pomodoroRound = 1; // Armazena o round do pomodoro, pois a cada 3 shortBreaks, vem 1 longBreak
char pomodoroClock[5]; // Armazena o timer do pomodoro formatado "99:99"
int  x, minX; // Usadas para movimentação horizontal de textos no display
TaskHandle_t pomodoroTimerHandle; // Handler do pomodoroTimer, que rodará em outro núcleo
eTaskState statusOf; // // Armazenará o status do pomodoroTimer, que rodará em outro núcleo
int active_mode; // Usado para armazenar o modo ativo no momento (relógio/pomodoro/motor)
// Modos disponíveis:
// 1: Clock
// 2: Pomodoro
// 3: Motor

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
  pomodoroRound += 1;
  pomodoroTimer = pomodoroWorkTime;
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

void scrollMessage() {
  display.setTextSize(1);
  display.setCursor(x, 0);
  display.print(notification);
  if (--x < minX) x = display.width();
}

void setup() {
  active_mode = 1;
  Serial.begin(serialSpeed);
  setupWifi();
  //setupBuzzer();
  setupMQTT();
  setupDisplay();
  mqttPublish(mqttResource, "ESP32 initialized", false);
  setupClock();
  getClockInfo();
  showClock();
  // xTaskCreatePinnedToCore(startPomodoroTimer, "startPomodoroTimer", 10000, NULL, 1, &pomodoroTimerHandle, 0);
}

void loop() {
  client.loop();
  display.clearDisplay();
  if (active_mode == 1) {
    // Modo relógio
    getClockInfo();
    showClock();
  } else if (active_mode == 2) {
    // Modo pomodoro timer
    showPomodoroTimer();
    statusOf = eTaskGetState(pomodoroTimerHandle);
    // Verifica se a task pomodoro já terminou
    if (statusOf == eReady) {
      active_mode = 1;
    }
  } else if (active_mode == 3) {
    // Modo rotor

  }
  scrollMessage();
  display.display();
}
