#ifndef CONSTANTS_H
#define CONSTANTS_H

#define ESP32#define RECEIVER true
// #define TRANSMITTER

#define CHANNEL 1 //127

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

////////////////////        DEBUG FLAGS
#define DEBUGMODE // Enable debug printing
#define DEBUGMODE_LOCAL
#define DEBUGMODE_CONTROL

#ifdef DEBUGMODE                                  //Macros are usually in all capital letters.
#define DPRINT(...) Serial.print(__VA_ARGS__)     //DPRINT is a macro, debug print
#define DPRINTF(...) Serial.printf(__VA_ARGS__)   //DPRINTF is a macro, debug print
#define DPRINTLN(...) Serial.println(__VA_ARGS__) //DPRINTLN is a macro, debug print with new line
#else
#define DPRINT(...) //now defines a blank line
#define DPRINTF(...)
#define DPRINTLN(...)
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