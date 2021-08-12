#include "Control.h"
#include "MQTT.h"
#include "constants.h"
#include "WiFi.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "Radio.h"
#include <nvs_flash.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"

void TaskBlink(void *pvParameters);
void TaskWifi(void *pvParameters);
void TaskServer();

WiFiClient espClient;
MQTT *mqttClient;
RF24 nrfClient(4, 5);

SemaphoreHandle_t sema_MQTT_KeepAlive;

TaskHandle_t blinkHandler, connectionHandler, mqttHandler, nrfHandler;

bool shouldConnectToInternet = false;
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

void onMqttMsg(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
  mqttClient->onMessage(topic, payload, properties, len, index, total);
}

void onMqttConnect(bool sessionPresent)
{
  mqttClient->onMqttConnect(sessionPresent);
}

void onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
  mqttClient->onMqttDisconnect(reason);
}

time_t setClock()
{
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  DPRINT("Waiting for NTP time sync: ");
  time_t NTPtime = time(nullptr);
  unsigned long start = millis();
  int timeout = 120 * 1000; //2 min
  while (NTPtime < 8 * 3600 * 2 && (unsigned long)(millis()) - start < timeout)
  {
    delay(250);
    DPRINT(".");
    NTPtime = time(nullptr);
  }
  DPRINTLN("");
  struct tm timeinfo;
  gmtime_r(&NTPtime, &timeinfo);
  DPRINT("Current time: ");
  DPRINT(asctime(&timeinfo));
  return NTPtime;
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
  Serial.printf("[WIFI] mode set to WIFI_STA %s\n", WiFi.mode(WIFI_STA) ? "OK" : "Failed!");
  WiFi.begin(SSID_LOCAL, PASSWORD_LOCAL);
  DPRINTLN("[WIFI] Connecting to " + String(SSID_LOCAL));
  for (;;)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      vTaskDelay(10000 / portTICK_PERIOD_MS);
      continue;
    }

    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_TIMEOUT_MS)
    {
      vTaskSuspend(blinkHandler);
      Control::led1(!Control::led1());
      vTaskDelay(250 / portTICK_RATE_MS);
    }

    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println("[WIFI] Failed ");
      shouldConnectToInternet = true;
      shouldConnectToServer = false;
      vTaskResume(blinkHandler);
      vTaskDelay(WIFI_RETRY_TIME_MS / portTICK_PERIOD_MS);
      continue;
    }

    Serial.println("[WIFI] Connected ");
    shouldConnectToInternet = false;
    shouldConnectToServer = true;
    TaskServer();
    // vTaskResume(mqttHandler);
  }
}

void MQTTkeepalive(void *pvParameters)
{
  // mqttClient->getPubSubClient()->setKeepAlive(90);
  // for (;;)
  // {
  //   if ((espClient.connected()) && (WiFi.status() == WL_CONNECTED))
  //   {
  //     xSemaphoreTake(sema_MQTT_KeepAlive, portMAX_DELAY); // whiles mqttClient.loop() is running no other mqtt operations should be in process
  //     mqttClient->getPubSubClient()->loop();
  //     xSemaphoreGive(sema_MQTT_KeepAlive);
  //   }
  //   else
  //   {
  //     Serial.println("[MQTT] Task Suspended");
  //     vTaskSuspend(mqttHandler);
  //   }
  //   vTaskDelay(250); //task runs approx every 250 mS
  // }
  // vTaskDelete(NULL);
}

void TaskNRF(void *pvParameters)
{
  for (;;)
  {
    if (true)
    {
    }
    else
    {
    }
  }
}

void TaskServer() //void *pvParameters
{
  setClock();
  Serial.println("[MQTT] Trying to connect to server ");
  Serial.println("[MQTT] " + (String)MQTT_HOST);
  Serial.printf("[MQTT] WiFi PORT: %d\n", MQTT_PORT);

  if (mqttClient->getMqttClient()->connected())
  {
    Serial.println("[MQTT] mqttClient-> disconnect ");
    mqttClient->getMqttClient()->disconnect();
  }

  if (mqttClient->connect())
  {
    Serial.println("[MQTT] Connected to server ");
    shouldConnectToServer = false;
    Control::led1(true);
    // vTaskResume(mqttHandler); //TODO:
    // Message message(NULL, message_type::message_type_CONFIRMATION, false);
    // mqttClient->send(TOPIC_CONFIRMATION_REPORT, &message);
  }
  else
  {
    Serial.println("[MQTT] Failed to connect to server");
    mqttClient->getMqttClient()->disconnect();
    vTaskResume(blinkHandler);
  }
}

void setup()
{
  // WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector

  Serial.begin(115200);
  uint8_t macAddress[6];
  WiFi.macAddress(macAddress);
  Configuration::generateSerial(macAddress);
  DPRINTLN(FIRMWARE_VERSION);
  Serial.println("\n\n[SETUP] EnergyIO ");
  DPRINTF("\n[SETUP] Serial: %s\n", Configuration::getSerial());

  esp_err_t ret = nvs_flash_init();
  switch (ret)
  {
  case ESP_OK:
  {
    Serial.println("[NVS_FLASH] ESP_OK");
    break;
  }
  case ESP_ERR_NVS_NO_FREE_PAGES:
  {
    Serial.println("[NVS_FLASH] ESP_ERR_NVS_NO_FREE_PAGES");
    break;
  }
  case ESP_ERR_NOT_FOUND:
  {
    Serial.println("[NVS_FLASH] ESP_ERR_NOT_FOUND");
    break;
  }
  default:;
  }

  Configuration::readFlash();
  nvs_stats_t nvs_stats;
  nvs_get_stats(NULL, &nvs_stats);

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
    DPRINTF("[SETUP] Wifi %d SSID: %s\tIntensity: %d", i, WiFi.SSID(i).c_str(), WiFi.RSSI(i));
    DPRINTLN();
  }
  Configuration::setWifisScan(wifisAmount, wifisIntensity, wifisSsid);

  mqttClient = new MQTT();

  mqttClient->getMqttClient()->onMessage(onMqttMsg);
  mqttClient->getMqttClient()->onConnect(onMqttConnect);
  mqttClient->getMqttClient()->onDisconnect(onMqttDisconnect);

  sema_MQTT_KeepAlive = xSemaphoreCreateBinary();
  xSemaphoreGive(sema_MQTT_KeepAlive);

  xTaskCreatePinnedToCore(TaskBlink, "TaskBlink", 1024, NULL, 1, &blinkHandler, 1);
  xTaskCreatePinnedToCore(TaskWifi, "TaskWifi", 1024 * 16, NULL, 3, &connectionHandler, 0);
  // xTaskCreatePinnedToCore(MQTTkeepalive, "MQTTkeepalive", 1024 * 16, NULL, 3, &mqttHandler, 1);
  // xTaskCreatePinnedToCore(TaskNRF, "TaskNRF", 1024, NULL, 2, &nrfHandler, 1);

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

  if (shouldConnectToServer)
    TaskServer();
}