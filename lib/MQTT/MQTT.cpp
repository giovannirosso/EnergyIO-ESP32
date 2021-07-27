#include "MQTT.h"
#include "constants.h"
#include "WString.h"

MQTT::MQTT(WiFiClient &espClient)
{
    // pubSubClient = new PubSubClient(MQTT_HOST, MQTT_PORT, espClient);
    this->pubSubClient.setClient(espClient);

    randomSeed(micros());
}

MQTT::~MQTT()
{
    this->pubSubClient.disconnect();
}

void MQTT::onMessage(String topic, byte *payload, unsigned int length)
{
    if (this->pubSubClient.connected())
    {
        Serial.println("pubSubClient.connected()");
        if (length > 0)
        {
            // if (message != NULL)
            // {
            //     delete message;
            //     Serial.println("Deleted");
            // }
        }
    }
}

bool MQTT::connect()
{
    if (!this->pubSubClient.connected())
    {
        Serial.printf("[MQTT] Attempting MQTT connection...");

        String pubSubClientId = CLIENT_ID;

        if (this->pubSubClient.connect(pubSubClientId.c_str(), MQTT_USER, MQTT_PASS))
        {
            Serial.println("[MQTT] Connected");

            subscribe(TOPIC_TEST_REQUEST);

            return true;
        }
        else
        {
            Serial.printf("[MQTT] failed, rc=");
            Serial.println(this->pubSubClient.state());
            return false;
        }
    }
}

// void MQTT::send(char *topic, Message *msg)
// {
//     String _topic = "device/" + "SERIAL" + "/" + (String)topic;
//     this->pubSubClient.publish(_topic.c_str(), msg->getMessage(), msg->getLength());
// }

void MQTT::subscribe(String topic)
{
    String _topic = "device/" + topic;
    this->pubSubClient.subscribe(_topic.c_str());
}

PubSubClient *MQTT::getPubSubClient()
{
    return &this->pubSubClient;
}