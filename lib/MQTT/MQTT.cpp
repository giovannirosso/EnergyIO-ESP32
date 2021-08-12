#include "MQTT.h"
#include "constants.h"
#include "WString.h"

void onMqttSubscribe(uint16_t packetId, uint8_t qos)
{
    Serial.println("Subscribe acknowledged.");
    Serial.print("  packetId: ");
    Serial.println(packetId);
    Serial.print("  qos: ");
    Serial.println(qos);
}

void onMqttPublish(uint16_t packetId)
{
    Serial.println("Publish acknowledged.");
    Serial.print("  packetId: ");
    Serial.println(packetId);
}

MQTT::MQTT()
{
    this->mqttClient.setKeepAlive(MQTT_KEEPALIVE);
    this->mqttClient.setServer(MQTT_HOST, MQTT_PORT);
    this->mqttClient.setCredentials(MQTT_USER, MQTT_PASS);
    this->mqttClient.onSubscribe(onMqttSubscribe);
    this->mqttClient.onPublish(onMqttPublish);
    // this->mqttClient.setSecure(MQTT_SECURE);
    // this->mqttClient.addServerFingerprint(mqttCertFingerprint);
}

MQTT::~MQTT()
{
    this->mqttClient.disconnect();
}

void MQTT::onMessage(char *topic, char *payload, AsyncMqttClientMessageProperties properties, size_t len, size_t index, size_t total)
{
    Serial.println("Publish received.");
    Serial.print("  topic: ");
    Serial.println(topic);
    Serial.print("  qos: ");
    Serial.println(properties.qos);
    Serial.print("  dup: ");
    Serial.println(properties.dup);
    Serial.print("  retain: ");
    Serial.println(properties.retain);
    Serial.print("  len: ");
    Serial.println(len);
    Serial.print("  index: ");
    Serial.println(index);
    Serial.print("  total: ");
    Serial.println(total);

    if (this->mqttClient.connected())
    {
        DPRINTLN("pubSubClient.connected()");
        if (len > 0)
        {
            Message *message = NULL;
            Message income(payload, len);
            if (String(topic) == "device/" + String() + "/" + TOPIC_TEST_REQUEST)
            {
            }
            else if (String(topic) == "device/" + String() + "/" + TOPIC_TEST_REQUEST)
            {
            }
            if (message != NULL)
            {
                delete message;
                DPRINTLN("Deleted");
            }
        }
    }
}

void MQTT::send(char *topic, Message *msg)
{
    String _topic = "device/" + (String)Configuration::getSerial() + "/" + (String)topic;
    this->mqttClient.publish(_topic.c_str(), 0, false, (const char *)msg->getMessage(), msg->getLength());
}

void MQTT::panelSend(char *topic, Message *msg)
{
    String _topic = "panel/" + (String)Configuration::getSerial() + "/" + (String)topic;
    this->mqttClient.publish(_topic.c_str(), 0, false, (const char *)msg->getMessage(), msg->getLength());
}

void MQTT::panelSubscribe(char *topic)
{
    String _topic = "panel/" + (String)Configuration::getSerial() + "/" + (String)topic;
    this->mqttClient.subscribe(_topic.c_str(), 0);
}

void MQTT::subscribe(char *topic)
{
    String _topic = "device/" + (String)Configuration::getSerial() + "/" + (String)topic;
    this->mqttClient.subscribe(_topic.c_str(), 0);
}

AsyncMqttClient *MQTT::getMqttClient()
{
    return &this->mqttClient;
}

void MQTT::onMqttConnect(bool sessionPresent)
{
    Serial.println("Connected to MQTT.");
    Serial.print("Session present: ");
    Serial.println(sessionPresent);
    subscribeALL();
}

void MQTT::onMqttDisconnect(AsyncMqttClientDisconnectReason reason)
{
    Serial.println("Disconnected from MQTT.");

    if (reason == AsyncMqttClientDisconnectReason::TLS_BAD_FINGERPRINT)
    {
        Serial.println("Bad server fingerprint.");
    }
}

void MQTT::subscribeALL()
{
    subscribe(TOPIC_TEST_REQUEST);
}

bool MQTT::connect()
{
    if (!this->mqttClient.connected())
    {
        String ClientId = DEVICE_TYPE;
        ClientId += "-";
        ClientId += String(Configuration::getSerial());
        Serial.println(ClientId.c_str());

        this->mqttClient.setClientId(ClientId.c_str());
        this->mqttClient.connect();

        unsigned long start = millis();
        while (!this->mqttClient.connected() && (unsigned long)(millis()) - start <= MQTT_SOCKET_TIMEOUT)
            vTaskDelay(250 / portTICK_RATE_MS);
        if (this->mqttClient.connected())
            return true;
        else
            return false;
    }
}