#include "Control.h"
#include "MQTT.h"
#include "constants.h"
#include "WiFi.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

void TaskBlink(void *pvParameters);
void TaskWifi(void *pvParameters);
void TaskServer(void *pvParameters);

WiFiClient espClient;
MQTT *mqttClient;

TaskHandle_t blinkHandler, connectionHandler, serverHandler;
bool shouldConnectToWifi = true;
bool shouldConnectToServer = false;

bool reset = false;
unsigned long debouncing_time = 200; //Debouncing Time in Milliseconds
unsigned long last_micros;
void IRAM_ATTR resetConfigs()
{
  if (((unsigned long)(millis() - last_micros)) >= debouncing_time)
  {
    reset = true;
    last_micros = millis();
  }
}

void IRAM_ATTR LED1_SW()
{
  if (((unsigned long)(millis() - last_micros)) >= debouncing_time)
  {
    Control::led1(!Control::led1());
    Serial.println("LED1");
    last_micros = millis();
  }
}

void IRAM_ATTR LED2_SW()
{
  if (((unsigned long)(millis() - last_micros)) >= debouncing_time)
  {
    Control::led2(!Control::led2());
    Serial.println("LED2");
    last_micros = millis();
  }
}

void TaskBlink(void *pvParameters) // This is a task.
{
  (void)pvParameters;

  for (;;)
  {
    if (WiFi.status() == WL_CONNECTED)
      vTaskSuspend(NULL);
    else
    {
      Control::led1(!Control::led1());
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
  }
}

void TaskWifi(void *pvParameters)
{
  for (;;)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      vTaskDelay(10000 / portTICK_PERIOD_MS);
      //Control::led1(true);
      continue;
    }

    Serial.println("[WIFI] Connecting ");
    Serial.printf("[WIFI] mode set to WIFI_STA %s\n", WiFi.mode(WIFI_STA) ? "OK" : "Failed!");
    WiFi.begin(SSID_LOCAL, PASSWORD_LOCAL);

    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_TIMEOUT_MS)
    {
    }

    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println("[WIFI] Failed ");
      shouldConnectToWifi = true;
      // vTaskResume(blinkHandler);
      vTaskDelay(WIFI_RETRY_TIME_MS / portTICK_PERIOD_MS);
      continue;
    }

    Serial.println("[WIFI] Connected ");
    mqttClient->getPubSubClient()->setClient(espClient);
    mqttClient->getPubSubClient()->setServer(MQTT_HOST, MQTT_PORT);
    shouldConnectToWifi = false;
    shouldConnectToServer = true;
  }
}

void callback(char *topic, byte *payload, unsigned int length)
{
  mqttClient->onMessage(topic, payload, length);
}

void TaskServer() //void *pvParameters
{
  Serial.printf("[MQTT] Trying to connect to server ");
  Serial.printf(MQTT_HOST);
  Serial.printf(": %d\n", MQTT_PORT);

  if (mqttClient->getPubSubClient()->connected())
  {
    Serial.println("[MQTT] mqttClient->getPubSubClient()->disconnect()");
    mqttClient->getPubSubClient()->disconnect();
  }

  if (mqttClient->connect())
  {
    Serial.println("[MQTT] Connected to server");
    shouldConnectToServer = false;
    mqttClient->getPubSubClient()->setCallback(callback);
    Control::led1(true);
  }
  else
  {
    Serial.println("[MQTT] Failed to connect to server");
    mqttClient->getPubSubClient()->disconnect();
    shouldConnectToServer = true;
    vTaskResume(blinkHandler);
  }
}

void setup()
{
  // WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  Serial.begin(115200);
  Serial.println("\n\n[SETUP] EnergyIO ");
  uint8_t macAddress[6];
  WiFi.macAddress(macAddress);

  Control::init();
  Control::led1(false);
  Control::led2(false);
  Control::led3(false);

  attachInterrupt(digitalPinToInterrupt(SW1), LED1_SW, FALLING);
  attachInterrupt(digitalPinToInterrupt(SW2), LED2_SW, FALLING);
  attachInterrupt(digitalPinToInterrupt(SW3), resetConfigs, FALLING);

  int wifisAmount = WiFi.scanNetworks();
  int *wifisIntensity = new int[wifisAmount];
  String *wifisSsid = new String[wifisAmount];
  for (int i = 0; i < wifisAmount; i++)
  {
    wifisSsid[i] = WiFi.SSID(i);
    wifisIntensity[i] = WiFi.RSSI(i);
    Serial.printf("[WIFI] %d SSID: %s\tIntensity: %d", i, WiFi.SSID(i).c_str(), WiFi.RSSI(i));
    Serial.println();
  }

  mqttClient = new MQTT(espClient);

  xTaskCreate(TaskBlink, "TaskBlink", 1024, NULL, 1, &blinkHandler);
  xTaskCreate(TaskWifi, "TaskWifi", 4096, NULL, 3, &connectionHandler);
  // xTaskCreate(TaskServer, "TaskServer", 1024, NULL, 2, &serverHandler);

  Serial.println("[SETUP] END\n");
}

void loop()
{
  if (reset)
  {
    unsigned long start = millis();
    int timeout = 5000;
    while (digitalRead(SW3) == LOW && (unsigned long)(millis()) - start <= timeout)
    {
      Serial.print(".");
      Control::led3(!Control::led3());
      delay(500);
    }
    if ((unsigned long)(millis()) - start > timeout)
    {
      Serial.println("[CONFIGS] Factory Reset");
      delay(500);
      ESP.restart();
    }
    reset = false;
  }

  if (!shouldConnectToWifi && shouldConnectToServer)
    TaskServer();

  if (WiFi.status() == WL_CONNECTED)
  {
    if (mqttClient->getPubSubClient()->connected())
    {
      mqttClient->getPubSubClient()->loop();
      yield();
    }
  }
}