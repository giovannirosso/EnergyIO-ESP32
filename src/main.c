#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h" //TODO: USAR ESSE

#include "nRF24.h"

#include <WifiStation.h>
#include <MqttClient.h>

#define CONFIG_BROKER_URL "mqtt://energyio.ml"
#define SSID "Liane_2G"
#define PASSWORD "055A64F7"
#define CLIENT_ID "MelloGay"
#define TEST_TOPIC "test"

// #define RECEIVER
#define TRANSMITTER
typedef union
{
    uint8_t value[4];
    unsigned long now_time;
} MYDATA_t;

#define CONFIG_CE_GPIO 4
#define CONFIG_CSN_GPIO 5
#define CONFIG_MISO_GPIO 19
#define CONFIG_MOSI_GPIO 23
#define CONFIG_SCLK_GPIO 18

MYDATA_t mydata;

#if defined(RECEIVER)
void receiver(void *pvParameters) //Reading
{
    NRF24_t dev;

    ESP_LOGI(pcTaskGetTaskName(0), "Start");
    ESP_LOGI(pcTaskGetTaskName(0), "CONFIG_CE_GPIO=%d", CONFIG_CE_GPIO);
    ESP_LOGI(pcTaskGetTaskName(0), "CONFIG_CSN_GPIO=%d", CONFIG_CSN_GPIO);
    spi_master_init(&dev, CONFIG_CE_GPIO, CONFIG_CSN_GPIO, CONFIG_MISO_GPIO, CONFIG_MOSI_GPIO, CONFIG_SCLK_GPIO);

    Nrf24_setRADDR(&dev, (uint8_t *)"FGHIJ");
    uint8_t payload = sizeof(mydata.now_time);
    uint8_t channel = 90;
    Nrf24_config(&dev, channel, payload);
    Nrf24_printDetails(&dev);
    ESP_LOGI(pcTaskGetTaskName(0), "Listening...");

    while (1)
    {
        if (Nrf24_dataReady(&dev))
        { //When the program is received, the received data is output from the serial port
            Nrf24_getData(&dev, mydata.now_time);
            ESP_LOGI(pcTaskGetTaskName(0), "Got data:%lu", mydata.now_time);
        }
        vTaskDelay(1);
    }
}
#endif

#if defined(TRANSMITTER)
void transmitter(void *pvParameters) //writing
{
    NRF24_t dev;

    ESP_LOGI(pcTaskGetTaskName(0), "Start");
    ESP_LOGI(pcTaskGetTaskName(0), "CONFIG_CE_GPIO=%d", CONFIG_CE_GPIO);
    ESP_LOGI(pcTaskGetTaskName(0), "CONFIG_CSN_GPIO=%d", CONFIG_CSN_GPIO);
    spi_master_init(&dev, CONFIG_CE_GPIO, CONFIG_CSN_GPIO, CONFIG_MISO_GPIO, CONFIG_MOSI_GPIO, CONFIG_SCLK_GPIO);

    Nrf24_setRADDR(&dev, (uint8_t *)"ABCDE");
    uint8_t payload = sizeof(mydata.value);
    uint8_t channel = 90;
    Nrf24_config(&dev, channel, payload);
    Nrf24_printDetails(&dev);

    while (1)
    {
        mydata.now_time = 123456789;//xTaskGetTickCount();
        Nrf24_setTADDR(&dev, (uint8_t *)"FGHIJ");  //Set the receiver address
        Nrf24_send(&dev, mydata.value);           //Send instructions, send random number value
        vTaskDelay(1);
        ESP_LOGI(pcTaskGetTaskName(0), "Wait for sending.....");
        if (Nrf24_isSend(&dev))
        {
            ESP_LOGI(pcTaskGetTaskName(0), "Send success:%lu", mydata.now_time);
        }
        else
        {
            ESP_LOGI(pcTaskGetTaskName(0), "Send fail:");
        }
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}
#endif

void onMessage(char *topic, char *data, int length)
{
    ESP_LOGI("main", "MQTT MESSAGE");

    printf("data: %s\n", data);
    printf("topic: %s\n", topic);
}

void app_main()
{
    //Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    wifiStationInit(SSID, PASSWORD, 10);

    if (wifiState() != CONNECTED)
        ESP_LOGI("main", "not connected to wifi");
    else
        ESP_LOGI("main", "connected to wifi");

    mqttInit("energyio.ml", 1883, CLIENT_ID);

    ESP_LOGI("main", "after mqtt init");

    setMqttCredentials("device", "HG7CrpAVuiLB7QD");
    setOnMessageCallback(onMessage);

    mqttStart();
    if (waitForMqttState() == MQTT_CONNECTED)
    {
        // vTaskDelay(1000 / portTICK_PERIOD_MS);

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    subscribe(TEST_TOPIC, 0);

        message msg = {
            .data = "asdasd",
            .topic = "giogay",
            .length = strlen("asdasd"),
        };

    publish(&msg);

#if defined(RECEIVER)
    // Create Task
    xTaskCreate(receiver, "RECV", 1024 * 2, NULL, 2, NULL);
#endif

#if defined(TRANSMITTER)
    // Create Task
    xTaskCreate(transmitter, "TRANS", 1024 * 2, NULL, 2, NULL);
#endif
}