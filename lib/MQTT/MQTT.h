#ifndef MQTT_H
#define MQTT_H

#include <constants.h>

#include <WiFiClient.h>
#include <PubSubClient.h>

#define MQTT_MAX_PACKET_SIZE 256

class MQTT
{
private:
    static char *host;
    static int port;
    PubSubClient pubSubClient;

public:
    MQTT(WiFiClient &espClient);

    // MQTT();
    ~MQTT();
    void onMessage(String topic, byte *payload, unsigned int length);
    bool connect();
    bool connect(String host, int port);
    // void send(char *topic, Message *msg);
    void subscribe(String topic);
    PubSubClient *getPubSubClient();
};

#endif //MQTT_H