#ifndef WIFI_STATION_H
#define WIFI_STATION_H

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"

#include "lwip/err.h"
#include "lwip/sys.h"

/* The event group allows multiple bits for each event, but we only care about two events:
 * - we are connected to the AP with an IP
 * - we failed to connect after the maximum amount of retries */
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

#define WIFI_STATION_DEBUG

#define EXAMPLE_ESP_WIFI_SSID "ALHN-6593"
#define EXAMPLE_ESP_WIFI_PASS "Vdb4U5k-XA"

typedef enum
{
    CONNECTED,
    NOT_CONNECTED,
    UNKNOWN
} WifiState;

typedef struct
{
    int maxRetries;
} EventHandlerArgs;

// FreeRTOS Signal handler
void wifiStationEventHandler(void *arg, esp_event_base_t event_base,
                             int32_t event_id, void *event_data);

/** Inits a Wifi Station 
 * @param ssid Network's SSID which the esp will connect
 * @param password Network's password which the esp will connect
 * @param retries Number of connection retries
 **/
void wifiStationInit(const char *ssid, const char *password, int retries);

/**
 * Blocks execution until it gets a state response from eventHandler
 */
WifiState wifiState();

#endif //WIFI_STATION_H