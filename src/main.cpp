#include "Control.h"
#include "constants.h"
#include "WiFi.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"

#define WIFI_TIMEOUT_MS 20000
#define WIFI_RETRY_TIME_MS 30000

void TaskBlink(void *pvParameters);
void TaskWifi(void *pvParameters);

TaskHandle_t blinkHandler, connectionHandler;
bool shouldConnectToWifi, shouldConnectToServer = true;

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

void IRAM_ATTR LED3_SW()
{
  if (((unsigned long)(millis() - last_micros)) >= debouncing_time)
  {
    Control::led3(!Control::led3());
    Serial.println("LED3");
    last_micros = millis();
  }
}

void TaskBlink(void *pvParameters) // This is a task.
{
  (void)pvParameters;

  for (;;)
  {
    Control::led1(!Control::led1());
    vTaskDelay(1000 / portTICK_PERIOD_MS);
  }
}

void TaskWifi(void *pvParameters)
{
  for (;;)
  {
    Serial.println("Wifi Task");
    if (WiFi.status() == WL_CONNECTED)
    {
      vTaskDelay(10000 / portTICK_PERIOD_MS);
      continue;
    }

    Serial.println("[WIFI] Connecting");
    WiFi.mode(WIFI_STA);
    WiFi.begin(SSID_LOCAL, PASSWORD_LOCAL);

    unsigned long startAttemptTime = millis();

    while (WiFi.status() != WL_CONNECTED &&
           millis() - startAttemptTime < WIFI_TIMEOUT_MS)
    {
    }

    if (WiFi.status() != WL_CONNECTED)
    {
      Serial.println("[WIFI] FAILED");
      vTaskDelay(WIFI_RETRY_TIME_MS / portTICK_PERIOD_MS);
      continue;
    }

    Serial.println("[WIFI] Connected: " + WiFi.localIP());
  }
}

bool connectWifi(int tries, int interval)
{
  Serial.println("Conectando ao Wifi");
  WiFi.begin(SSID_LOCAL, PASSWORD_LOCAL);

  int i = 0;
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.printf(".");
    Control::led1(!Control::led1());
    delay(interval);
    yield();
    if (i++ > tries)
    {
      WiFi.disconnect(true);
      return false;
    }
  }
  Serial.println("Conectou WIFI");
  shouldConnectToWifi = false;
  return true;
}

bool connectInternet(int tries, int interval)
{
  bool connectedToInternet = connectWifi(tries, interval);
  if (connectedToInternet)
  {
    //MQTT
  }
  else
  {
    Serial.println("Não conectou internet");
  }

  return connectedToInternet;
}

void setup()
{
  // WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  Serial.begin(115200);
  Serial.println("\n\n");
  uint8_t macAddress[6];
  WiFi.macAddress(macAddress);

  Control::init();
  Control::led1(true);
  Control::led2(false);
  Control::led3(true);

  attachInterrupt(digitalPinToInterrupt(SW1), LED1_SW, FALLING);
  attachInterrupt(digitalPinToInterrupt(SW2), LED2_SW, FALLING);
  attachInterrupt(digitalPinToInterrupt(SW3), LED3_SW, FALLING);

  Serial.printf("Wi-Fi mode set to WIFI_STA %s\n", WiFi.mode(WIFI_STA) ? "OK" : "Failed!");
  int wifisAmount = WiFi.scanNetworks();
  int *wifisIntensity = new int[wifisAmount];
  String *wifisSsid = new String[wifisAmount];
  for (int i = 0; i < wifisAmount; i++)
  {
    wifisSsid[i] = WiFi.SSID(i);
    wifisIntensity[i] = WiFi.RSSI(i);
    Serial.printf("Wifi %d SSID: %s\tIntensity: %d", i, WiFi.SSID(i).c_str(), WiFi.RSSI(i));
    Serial.println();
  }

  connectInternet(100, 250);

  xTaskCreate(TaskBlink, "TaskBlink", 1024, NULL, 1, &blinkHandler);
  // xTaskCreate(TaskWifi, "TaskWifi", 5120, NULL, 2, &connectionHandler);

  Serial.println("END SETUP");
}

void loop()
{
}