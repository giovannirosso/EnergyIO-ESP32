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

void TaskBlink(void *pvParameters);
void TaskWifi(void *pvParameters);
//void TaskSoftAp(void *pvParameters);

MQTT *mqttClient;
RADIO *nrfClient;

Ticker mqttReconnectTicker;

SemaphoreHandle_t sema_SoftAP;

TaskHandle_t blinkHandler, wifiHandler, nrfHandler;

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
  DPRINTLN("");
  struct tm timeinfo;
  gmtime_r(&NTPtime, &timeinfo);
  DPRINT("[NTP TIME] Current time: ");
  DPRINT(asctime(&timeinfo));
  return NTPtime;
}

void TaskBlink(void *pvParameters)
{
  (void)pvParameters;

  for (;;)
  {
    if (WiFi.status() == WL_CONNECTED && mqttClient->connected())
    {
      DPRINTLN("[LED] Task Suspended");
      vTaskSuspend(NULL);
    }
    else
    {
      Control::led1(!Control::led1());
      vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
  }
}

void WifiDisconnected(WiFiEvent_t event)
{
  DPRINTLN("[WIFI] Disconnected from WiFi");
  mqttClient->disconnect();
  WiFi.removeEvent(WifiDisconnected, SYSTEM_EVENT_STA_DISCONNECTED);
  vTaskResume(wifiHandler);
  vTaskResume(blinkHandler);
}

void WifiGotIp(WiFiEvent_t event)
{
  DPRINT("[WIFI] Obtained IP address: ");
  DPRINTLN(WiFi.localIP());
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
      DPRINTLN("[WIFI] Task Suspended");
      vTaskSuspend(NULL);
      // vTaskDelay(10000 / portTICK_PERIOD_MS);
    }

    WiFi.begin(Configuration::getLocalSsid(), Configuration::getLocalPassword());
    DPRINTLN("[WIFI] Connecting to " + String(Configuration::getLocalSsid()));
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
      DPRINTLN("[WIFI] Connection failed, retry in 60s");
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
    //server.loop();
    xSemaphoreGive(sema_SoftAP);
    vTaskDelay(2 / portTICK_RATE_MS);
  }
  // vTaskDelete(NULL);
}

void TaskNRF(void *pvParameters)
{
  nrfClient->init();
  for (;;)
  {
    nrfClient->listen();
    vTaskDelay(500 / portTICK_RATE_MS);
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

  //DPRINTF("[LOCAL] mode set to WIFI_AP %s\n", WiFi.mode(WIFI_MODE_AP) ? "OK" : "Failed!");
  //server.connect(Configuration::getApSsid(), Configuration::getApPassword());
  //DPRINTF("[LOCAL] MODE: %d\n", WiFi.getMode());

  mqttClient = new MQTT();
  nrfClient = new RADIO();

  sema_SoftAP = xSemaphoreCreateBinary();
  xSemaphoreGive(sema_SoftAP);

  xTaskCreatePinnedToCore(TaskBlink, "TaskBlink", 1024, NULL, 1, &blinkHandler, 1);
  xTaskCreatePinnedToCore(TaskWifi, "TaskWifi", 1024 * 16, NULL, 5, &wifiHandler, 0);
  //xTaskCreatePinnedToCore(TaskSoftAp, "TaskSoftAp", 1024 * 4, NULL, 4, NULL, 1);
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
      delay(500);
      ESP.restart();
    }
    reset = false;
  }

  if (Serial.available() > 0)
  {
    String income = Serial.readString();
    if (income == "getSerial")
    {
      DPRINTLN(Configuration::getSerial());
    }
    else if (income == "reset")
    {
      Control::led1(false);
      DPRINTLN("[RESET]");
      Configuration::reset();
      delay(500);
      ESP.restart();
    }
    else
    {
      DPRINTLN(income);
      DPRINTLN("is NOT CODED");
    }
  }
}