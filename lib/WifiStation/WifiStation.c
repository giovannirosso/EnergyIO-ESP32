#include "WifiStation.h"

/* FreeRTOS event group to signal when we are connected*/
static EventGroupHandle_t s_wifi_event_group;

static int s_retry_num = 0;

static esp_event_handler_instance_t instance_any_id;
static esp_event_handler_instance_t instance_got_ip;

#ifdef WIFI_STATION_DEBUG
static const char *TAG = "wifi station";
#endif

void wifiStationEventHandler(void *arg, esp_event_base_t event_base,
                             int32_t event_id, void *event_data)
{
    EventHandlerArgs *args = (EventHandlerArgs *)arg;

    ESP_LOGI(TAG, "max retries: %d", args->maxRetries);

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        esp_wifi_connect();
    }
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        if (s_retry_num < args->maxRetries)
        {
            esp_wifi_connect();
            s_retry_num++;

#ifdef WIFI_STATION_DEBUG
            ESP_LOGI(TAG, "retry to connect to the AP");
#endif
        }
        else
        {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
#ifdef WIFI_STATION_DEBUG
            ESP_LOGI(TAG, "connect to the AP fail");
#endif
        }
    }
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
#ifdef WIFI_STATION_DEBUG
        ESP_LOGI(TAG, "got ip:" IPSTR, IP2STR(&event->ip_info.ip));
#endif
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifiStationInit(const char *ssid, const char *password, int retries)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());

    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    EventHandlerArgs args = {
        .maxRetries = retries};

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifiStationEventHandler, &args, &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &wifiStationEventHandler,
                                                        &args,
                                                        &instance_got_ip));

    wifi_config_t wifi_config;
    memset(&wifi_config, 0, sizeof(wifi_config_t));
    strcpy((char *)wifi_config.sta.ssid, ssid);
    wifi_config.sta.scan_method = WIFI_ALL_CHANNEL_SCAN; //force full scan to be able to choose the nearest / strongest AP

    if (password)
    {
        if (strlen(password) == 64)
        { // it's not a passphrase, is the PSK
            memcpy((char *)wifi_config.sta.password, password, 64);
        }
        else
        {
            strcpy((char *)wifi_config.sta.password, password);
        }
    }

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

#ifdef WIFI_STATION_DEBUG
    ESP_LOGI(TAG, "wifi config ssid: %s password: %s", wifi_config.sta.ssid, wifi_config.sta.password);
    ESP_LOGI(TAG, "wifi_init_sta finished.");
#endif
}

WifiState wifiState()
{
    /* Waiting until either the connection is established (WIFI_CONNECTED_BIT) or connection failed for the maximum
     * number of re-tries (WIFI_FAIL_BIT). The bits are set by event_handler() (see above) */
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    /* xEventGroupWaitBits() returns the bits before the call returned, hence we can test which event actually
     * happened. */
    if (bits & WIFI_CONNECTED_BIT)
    {
#ifdef WIFI_STATION_DEBUG
        ESP_LOGI(TAG, "Connected to Wifi");
#endif
        return CONNECTED;
    }
    else if (bits & WIFI_FAIL_BIT)
    {
#ifdef WIFI_STATION_DEBUG
        ESP_LOGI(TAG, "Failed to connecet to Wifi");
#endif
        return NOT_CONNECTED;
    }
    else
    {
#ifdef WIFI_STATION_DEBUG
        ESP_LOGE(TAG, "UNEXPECTED EVENT");
#endif
        return UNKNOWN;
    }

    /* The event will not be processed after unregister */
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, instance_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(WIFI_EVENT, ESP_EVENT_ANY_ID, instance_any_id));
    vEventGroupDelete(s_wifi_event_group);
}