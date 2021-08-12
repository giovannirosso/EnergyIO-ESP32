#ifndef MQTT_H
#define HMQTT_H
#include <Arduino.h>
#include <constants.h>

#include <WiFiClient.h>
#include <AsyncMqttClient.h>
#include "Message.h"
#include "Configuration.h"
class MQTT
{
private:
    static char *host;
    static int port;
    AsyncMqttClient mqttClient;
    bool connected;

public:
    MQTT();

    // MQTT();
    ~MQTT();
    void onMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total);
    void onMqttConnect(bool sessionPresent);
    void onMqttDisconnect(AsyncMqttClientDisconnectReason reason);
    // void onMqttSubscribe(uint16_t packetId, uint8_t qos);
    // void onMqttPublish(uint16_t packetId);
    bool connect();
    void send(char *topic, Message *msg);
    void subscribe(char *topic);
    void subscribeALL();
    void panelSend(char *topic, Message *msg);
    void panelSubscribe(char *topic);
    AsyncMqttClient *getMqttClient();
    void OTAReturnCallback(int user, bool status);
};

#endif //MQTT_H