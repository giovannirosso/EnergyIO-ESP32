#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <Arduino.h>

#define ESP32
#define RECEIVER true
// #define TRANSMITTER

#define FIRMWARE_VERSION "v1.0"

#define WIFI_RETRY_TIME_MS 300000
#define WIFI_CONNECTION_STATUS 60000
#define WIFI_TIMEOUT_MS 15000

#define MQTT_MAX_PACKET_SIZE 256
#define MQTT_SOCKET_TIMEOUT 10000
#define MQTT_RETRY_TIME_MS 15000
#define MQTT_CONNECTION_TIMEOUT 10000
#define MQTT_KEEPALIVE 30

#define WARMUP_TIME_MS 10000

#define MQTT_HOST "energyio.ml"
#define MQTT_PORT 1883
#define MQTT_USER "device"
#define MQTT_PASS "HG7CrpAVuiLB7QD"

#define CLIENT_ID "HubModule"

#define SSID_LOCAL "Rosso"
#define PASSWORD_LOCAL "055A64F7"

#define TOPIC_REGISTER_REQUEST "register/request"
#define TOPIC_REGISTER_REPORT "register/report"
#define TOPIC_SENSOR_REGISTER "sensor/register"

#define SERVER_PORT 80

////////////////////        DEBUG FLAGS
#define DEBUGMODE // Enable debug printing
#define DEBUGMODE_LOCAL
#define DEBUGMODE_CONTROL

#ifdef DEBUGMODE                                  // Macros are usually in all capital letters.
#define DPRINT(...) Serial.print(__VA_ARGS__)     // DPRINT is a macro, debug print
#define DPRINTF(...) Serial.printf(__VA_ARGS__)   // DPRINTF is a macro, debug print
#define DPRINTLN(...) Serial.println(__VA_ARGS__) // Serial.println is a macro, debug print with new line
#else
#define DPRINT(...) // now defines a blank line
#define DPRINTF(...)
#define Serial .println(...)
#endif

#ifdef DEBUGMODE_LOCAL
#define DLPRINT(...) Serial.print(__VA_ARGS__)
#define DLPRINTF(...) Serial.printf(__VA_ARGS__)
#define DLPRINTLN(...) Serial.println(__VA_ARGS__)
#else
#define DLPRINT(...)
#define DLPRINTF(...)
#define DLPRINTLN(...)
#endif

#ifdef DEBUGMODE_CONTROL
#define DCPRINT(...) Serial.print(__VA_ARGS__)
#define DCPRINTF(...) Serial.printf(__VA_ARGS__)
#define DCPRINTLN(...) Serial.println(__VA_ARGS__)
#else
#define DCPRINT(...)
#define DCPRINTF(...)
#define DCPRINTLN(...)
#endif

#endif