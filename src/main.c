/* WiFi softAP Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "esp_system.h"

#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h" //TODO: USAR ESSE

#include <WifiStation.h>
#include <MqttClient.h>

#define CONFIG_BROKER_URL "mqtt://energyio.ml"

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

    wifiStationInit("ALHN-6593", "Vdb4U5k-XA", 10);

    if (wifiState() != CONNECTED)
        ESP_LOGI("main", "not connected to wifi");
    else
        ESP_LOGI("main", "connected to wifi");

    mqttInit("energyio.ml", 1883, "giogay");

    ESP_LOGI("main", "after mqtt init");

    setMqttCredentials("device", "HG7CrpAVuiLB7QD");
    setOnMessageCallback(onMessage);

    mqttStart();
    if (waitForMqttState() == MQTT_CONNECTED)
    {
        // vTaskDelay(1000 / portTICK_PERIOD_MS);

        subscribe("lidoburro", 0);

        message msg = {
            .data = "asdasd",
            .topic = "giogay",
            .length = strlen("asdasd"),
        };

        publish(&msg);
    }
}