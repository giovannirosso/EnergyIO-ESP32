#include "Control.h"
#include "MQTT.h"
#include "constants.h"
#include "Ticker.h"
#include "WiFi.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "Radio.h"
#include <nvs_flash.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_task_wdt.h"
#include "Local.h"
#include "SPIFFS.h"

void TaskBlink(void *pvParameters);
void TaskWifi(void *pvParameters);
// void TaskSoftAp(void *pvParameters);

MQTT *mqttClient;
RADIO *nrfClient;
Local *localServer;

Ticker mqttReconnectTicker;

SemaphoreHandle_t sema_SoftAP;

TaskHandle_t blinkHandler, wifiHandler, nrfHandler;

unsigned long debouncing_time = 200; // Debouncing Time in Milliseconds
unsigned long last_micros;

bool reset = false;
void IRAM_ATTR resetConfigs()
{
  if (((unsigned long)(millis() - last_micros)) >= debouncing_time)
  {
    reset = true;
    last_micros = millis();
  }
}

bool changeRole = false;
void IRAM_ATTR changeRoleSW()
{
  if (((unsigned long)(millis() - last_micros)) >= debouncing_time)
  {
    Serial.println("changeRoleSW");
    changeRole = true;
    last_micros = millis();
  }
}

bool initPairing = false;
void IRAM_ATTR pairingButton()
{
  if (((unsigned long)(millis() - last_micros)) >= debouncing_time)
  {
    Serial.println("pairingButton");
    initPairing = true;
    last_micros = millis();
  }
}

time_t NTPtime;
struct tm timeinfo;
time_t setClock()
{
  configTime(3 * 3600, 0, "pool.ntp.org", "time.nist.gov");
  DPRINT("[NTP TIME] Waiting for NTP time sync: ");
  time_t NTPtime = time(nullptr);
  unsigned long start = millis();
  int timeout = 30 * 1000;
  while (NTPtime < 8 * 3600 * 2 && (unsigned long)(millis()) - start < timeout)
  {
    delay(250);
    DPRINT(".");
    NTPtime = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&NTPtime, &timeinfo);
  DPRINT("[NTP TIME] Current time: ");
  DPRINT(asctime(&timeinfo));
  return NTPtime;
}

unsigned long getTime()
{
  if (!getLocalTime(&timeinfo))
    NTPtime = setClock();

  time(&NTPtime);
  return NTPtime;
}

void TaskBlink(void *pvParameters)
{
  (void)pvParameters;

  for (;;)
  {
    if (WiFi.status() == WL_CONNECTED && mqttClient->connected())
    {
      Serial.println("[LED] Task Suspended");
      vTaskSuspend(NULL);
    }
    else
    {
      Control::led1(!Control::led1());
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
  }
}

void onApConnection(WiFiEvent_t event)
{
  vTaskSuspend(wifiHandler);
  DPRINTLN("[WIFI] Ap Client Connected");
}

void onApDisconnection(WiFiEvent_t event)
{
  vTaskResume(wifiHandler);
  DPRINTLN("[WIFI] Ap Client Disconnected");
}

void WifiDisconnected(WiFiEvent_t event)
{
  Serial.println("[WIFI] Disconnected from WiFi");
  mqttClient->disconnect();
  WiFi.removeEvent(WifiDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);
  vTaskResume(wifiHandler);
  vTaskResume(blinkHandler);
}

void WifiGotIp(WiFiEvent_t event)
{
  DPRINT("[WIFI] Obtained IP address: ");
  Serial.println(WiFi.localIP());
  setClock();
  esp_mqtt_client_reconnect(mqttClient->getClient());
  WiFi.onEvent(WifiDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);
}

void TaskWifi(void *pvParameters)
{
  WiFi.onEvent(WifiGotIp, SYSTEM_EVENT_STA_GOT_IP);
  // DPRINTF("[WIFI] mode set to WIFI_STA %s\n", WiFi.mode(WIFI_MODE_STA) ? "OK" : "Failed!");
  for (;;)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("\n[WIFI] Task Suspended");
      vTaskSuspend(NULL);
      // vTaskDelay(10000 / portTICK_PERIOD_MS);
    }

    WiFi.begin(Configuration::getLocalSsid(), Configuration::getLocalPassword());
    Serial.println("[WIFI] Connecting to " + String(Configuration::getLocalSsid()));
    DPRINTF("[WIFI] MODE: %d\n", WiFi.getMode());
    vTaskSuspend(blinkHandler);

    unsigned long startAttemptTime = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < WIFI_TIMEOUT_MS)
    {
      Control::led1(!Control::led1());
      vTaskDelay(250 / portTICK_RATE_MS);
    }
    vTaskResume(blinkHandler);

    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println("[WIFI] Connection failed, retry in 60s");
      mqttClient->disconnect();
      vTaskDelay(WIFI_RETRY_TIME_MS / portTICK_PERIOD_MS);
    }
  }
  // vTaskDelete(NULL);
}

void TaskSoftAp(void *pvParameters)
{
  // server.connect(Configuration::getApSsid(), Configuration::getApPassword());
  for (;;)
  {
    xSemaphoreTake(sema_SoftAP, portMAX_DELAY); // whiles server.loop() is running no other mqtt operations should be in process
    localServer->handle();
    xSemaphoreGive(sema_SoftAP);
    vTaskDelay(2 / portTICK_RATE_MS);
  }
  // vTaskDelete(NULL);
}

void TaskNRF(void *pvParameters)
{
  nrfClient->init();
  int aux = 0;
  vTaskDelay(WARMUP_TIME_MS / portTICK_RATE_MS);
  Serial.println("[NRF] Init");

  for (;;)
  {
    if (nrfClient->getRole() && initPairing == false)
      aux = nrfClient->listen();
    if (aux == 1 && mqttClient->isConnected) //?WATER
    {
      Message message(Configuration::get_instantMeasure(), getTime());
      mqttClient->sensorReport(&message, SensorType_WATER);
      Configuration::setLastPipe(NULL);
    }
    else if (aux == 2 && mqttClient->isConnected) //?EnERGY
    {
      Message message(Configuration::get_v_rms(), Configuration::get_i_rms(), Configuration::get_pot_ativa(), Configuration::get_pot_aparente(), getTime());
      mqttClient->sensorReport(&message, SensorType_ENERGY);
      Configuration::setLastPipe(NULL);
    }
    vTaskDelay(5 / portTICK_RATE_MS);
  }
}

void setup()
{
  // WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector

  Serial.begin(115200);
  SPIFFS.begin(true);

  uint8_t macAddress[6];
  WiFi.macAddress(macAddress);
  Configuration::generateSerial(macAddress);
  Serial.println(FIRMWARE_VERSION);
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

  attachInterrupt(digitalPinToInterrupt(SW1), changeRoleSW, FALLING);
  attachInterrupt(digitalPinToInterrupt(SW2), pairingButton, FALLING);
  attachInterrupt(digitalPinToInterrupt(SW3), resetConfigs, FALLING);

  int wifisAmount = WiFi.scanNetworks();
  int *wifisIntensity = new int[wifisAmount];
  String *wifisSsid = new String[wifisAmount];
  for (int i = 0; i < wifisAmount; i++)
  {
    wifisSsid[i] = WiFi.SSID(i);
    wifisIntensity[i] = WiFi.RSSI(i);
    DPRINTF("[SETUP] Wifi %d SSID: %s\tIntensity: %d", i, WiFi.SSID(i).c_str(), WiFi.RSSI(i));
    Serial.println();
  }
  Configuration::setWifisScan(wifisAmount, wifisIntensity, wifisSsid);

  DPRINTF("[LOCAL] mode set to WIFI_AP %s\n", WiFi.mode(WIFI_MODE_AP) ? "OK" : "Failed!");
  localServer = new Local(Configuration::getApSsid(), Configuration::getApPassword());
  WiFi.onEvent(onApConnection, SYSTEM_EVENT_AP_STACONNECTED);
  WiFi.onEvent(onApDisconnection, SYSTEM_EVENT_AP_STADISCONNECTED);

  mqttClient = new MQTT();
  nrfClient = new RADIO();

  sema_SoftAP = xSemaphoreCreateBinary();
  xSemaphoreGive(sema_SoftAP);

  xTaskCreatePinnedToCore(TaskBlink, "TaskBlink", 1024, NULL, 1, &blinkHandler, 1);
  xTaskCreatePinnedToCore(TaskWifi, "TaskWifi", 1024 * 16, NULL, 5, &wifiHandler, 0);
  xTaskCreatePinnedToCore(TaskSoftAp, "TaskSoftAp", 1024 * 4, NULL, 4, NULL, 1);
  xTaskCreatePinnedToCore(TaskNRF, "TaskNRF", 1024 * 4, NULL, 4, &nrfHandler, 1);

  mqttClient->init(MQTT_HOST, MQTT_PORT, MQTT_USER, MQTT_PASS);

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
      Configuration::reset();
      delay(500);
      ESP.restart();
    }
    reset = false;
  }

  if (changeRole)
  {
    nrfClient->changeRole(!nrfClient->getRole());
    changeRole = false;
  }

  if (initPairing)
  {
    unsigned long start = millis();
    bool aux = false;
    nrfClient->changeRole(false);
    while (aux != true && (unsigned long)(millis()) - start <= 15000)
    {
      if (nrfClient->pairingMode())
      {
        aux = true;
      }
      Control::led3(!Control::led3());
      delay(250);
    }
    if ((unsigned long)(millis()) - start > 15000)
    {
      Serial.println("[PAIRING] Timeout");
      nrfClient->changeRole(true);
    }
    start = millis();
    while (aux == true && (unsigned long)(millis()) - start <= 15000)
    {
      if (nrfClient->listenPairing())
      {
        aux = false;
        String Serial = Configuration::getLastRegistered();
        SensorType type;
        if (Serial[0] == 'W')
          type = SensorType_WATER;
        else if (Serial[0] == 'E')
          type = SensorType_ENERGY;
        else
          return;
        Message message(Serial, type);
        mqttClient->send(TOPIC_SENSOR_REGISTER, &message);
        delay(1000);
        ESP.restart();
      }
      Control::led3(!Control::led3());
      delay(50);
    }
    if ((unsigned long)(millis()) - start > 15000)
    {
      Serial.println("[PAIRING] Timeout");
    }
    initPairing = false;
  }

  if (Serial.available() > 0)
  {
    String income = Serial.readString();
    if (income == "getSerial")
    {
      Serial.println(Configuration::getSerial());
    }
    else if (income == "reset")
    {
      Control::led1(false);
      Serial.println("[RESET]");
      Configuration::reset();
      delay(500);
      ESP.restart();
    }
    else if (income == "se")
    {
      Serial.println(Configuration::getSensorInPipeSerial());
    }
    else
    {
      Serial.println(income);
      Serial.println("is NOT CODED");
    }
  }
}