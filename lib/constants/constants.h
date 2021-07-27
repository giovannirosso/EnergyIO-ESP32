#ifndef CONSTANTS_H
#define CONSTANTS_H

#define ESP32
// #define RECEIVER
// #define TRANSMITTER

#define WIFI_RETRY_TIME_MS 30000
#define WIFI_CONNECTION_STATUS 60000
#define WIFI_TIMEOUT_MS 15000

#define MQTT_HOST "energyio.ml"
#define MQTT_PORT 1883
#define MQTT_USER "device"
#define MQTT_PASS "HG7CrpAVuiLB7QD"

#define CLIENT_ID "MasterModule"

#define SSID_LOCAL "Rosso"
#define PASSWORD_LOCAL "055A64F7"

#define TOPIC_TEST_REQUEST "test/request"
#define TOPIC_TEST_REPORT "test/report"

#endif