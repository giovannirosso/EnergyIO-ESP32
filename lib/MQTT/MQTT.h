#ifndef MQTT_H
#define HMQTT_H
#include <Arduino.h>
#include <constants.h>

#include <WiFiClient.h>
#include "Message.h"
#include "Configuration.h"

#include "mqtt_client.h"

#define TAG "MINDTECH_MQTT"
class MQTT
{
private:
    esp_mqtt_client_handle_t mqttClient;

public:
    MQTT();
    ~MQTT();

    static bool isConnected;
    bool connected();
    void disconnect();

    void init(const char *host, uint16_t port, const char *mqtt_user, const char *mqtt_pass);

    void send(char *topic, Message *msg);
    void subscribe(char *topic);
    void subscribeALL();

    esp_mqtt_client_handle_t getClient();
};

#endif //MQTT_H