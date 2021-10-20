#include "MQTT.h"
#include "constants.h"
#include "WString.h"
#include "Control.h"
#include "Configuration.h"

bool MQTT::isConnected = false;

MQTT::MQTT()
{
}

MQTT::~MQTT()
{
    this->disconnect();
    esp_mqtt_client_destroy(this->mqttClient);
}

bool MQTT::connected()
{
    return isConnected;
}

void MQTT::disconnect()
{

    esp_mqtt_client_disconnect(this->mqttClient);
}

void MQTT::send(char *topic, Message *msg)
{
    if (!this->isConnected)
        return;
    int msg_id;
    String _topic = "hub/" + (String)Configuration::getSerial() + "/" + (String)topic;
    msg_id = esp_mqtt_client_publish(mqttClient, _topic.c_str(), (const char *)msg->getMessage(), msg->getLength(), 0, 0);
    ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
}

void MQTT::sensorReport(Message *msg, SensorType type)
{
    if (!this->isConnected)
        return;
    int msg_id;
    String _topic;
    if (type == SensorType_ENERGY)
        _topic = "sensor/" + Configuration::getSensorSerial() + "/energy/report";
    else if (type == SensorType_WATER)
        _topic = "sensor/" + Configuration::getSensorSerial() + "/water/report";
    DPRINTLN(_topic);
    msg_id = esp_mqtt_client_publish(mqttClient, _topic.c_str(), (const char *)msg->getMessage(), msg->getLength(), 0, 0);
    ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
}

void MQTT::subscribe(char *topic)
{
    int msg_id;
    String _topic = "hub/" + (String)Configuration::getSerial() + "/" + (String)topic;
    msg_id = esp_mqtt_client_subscribe(mqttClient, _topic.c_str(), 0);
    ESP_LOGI(TAG, "sent subscribe successful, msg_id=%d", msg_id);
}

void MQTT::subscribeALL()
{
    subscribe(TOPIC_REGISTER_REQUEST);
    DPRINTLN("[MQTT] Subscribed all");
}

static void onMqttConnect(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    MQTT *self = static_cast<MQTT *>(handler_args);
    DPRINTLN("[MQTT] MQTT_EVENT_CONNECTED");
    self->isConnected = true;
    self->subscribeALL();

    Control::led1(true);
    // Message message(2499, 123456789, DataType::DataType_ANY_DATA);
    // self->send(TOPIC_TEST_REPORT, &message);
}

static void onMqttDisconnect(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    MQTT *self = static_cast<MQTT *>(handler_args);
    DPRINTLN("[MQTT] MQTT_EVENT_DISCONNECTED");
    self->disconnect();
}

static void onMessage(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    DPRINTLN("[MQTT] MQTT_EVENT_DATA");
    MQTT *self = static_cast<MQTT *>(handler_args);
    esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t)event_data;

    if (self->isConnected)
    {
        if (event->data_len > 0)
        {
            Message *message = NULL;
            Message income(event->data, event->data_len);
            if (strncmp(event->topic, String("hub/" + String(Configuration::getSerial()) + "/" + TOPIC_REGISTER_REQUEST).c_str(), event->topic_len) == 0)
            {
                DPRINTLN(event->topic);
                String macAddress = WiFi.macAddress();
                Serial.println(macAddress);
                Serial.println(sizeof(macAddress));
                char mac[32];
                strcpy(mac, macAddress.c_str());
                message = new Message(mac);
                self->send(TOPIC_REGISTER_REPORT, message);
            }
            // else if (String(topic) == "device/" + String() + "/" + TOPIC_TEST_REQUEST)
            // {
            // }
            if (message != NULL)
            {
                delete message;
                DPRINTLN("Deleted");
            }
        }
    }
}

esp_mqtt_client_handle_t MQTT::getClient()
{
    return this->mqttClient;
}

void MQTT::init(const char *host, uint16_t port, const char *mqtt_user, const char *mqtt_pass)
{
    DPRINTLN("[MQTT] Trying to connect to server ");
    DPRINTLN("[MQTT] " + (String)host);
    DPRINTF("[MQTT] MQTT PORT: %d\n", port);

    String ClientId;
    ClientId += String(Configuration::getSerial());

    const esp_mqtt_client_config_t mqtt_cfg = {
        .event_handle = NULL,
        .event_loop_handle = NULL,
        .host = host,
        .uri = NULL,
        .port = port,
        .client_id = ClientId.c_str(),
        .username = mqtt_user,
        .password = mqtt_pass,
        .lwt_topic = NULL,
        .lwt_msg = NULL,
        .lwt_qos = NULL,
        .lwt_retain = NULL,
        .lwt_msg_len = NULL,
        .disable_clean_session = NULL,
        .keepalive = 60,
        .disable_auto_reconnect = NULL,
        .user_context = NULL,
        .task_prio = 5,
        .task_stack = 1024 * 10,
        .buffer_size = 1024,
        .cert_pem = NULL,
        .cert_len = 0,
        .client_cert_pem = NULL,
        .client_cert_len = NULL,
        .client_key_pem = NULL,
        .client_key_len = NULL,
        .transport = MQTT_TRANSPORT_OVER_TCP,
    };

    ESP_LOGI(TAG, "[APP] Free memory: %d bytes", esp_get_free_heap_size());
    this->mqttClient = esp_mqtt_client_init(&mqtt_cfg);

    esp_mqtt_client_register_event(this->mqttClient, MQTT_EVENT_CONNECTED, onMqttConnect, (void *)this);
    esp_mqtt_client_register_event(this->mqttClient, MQTT_EVENT_DISCONNECTED, onMqttDisconnect, (void *)this);
    esp_mqtt_client_register_event(this->mqttClient, MQTT_EVENT_DATA, onMessage, (void *)this);

    esp_mqtt_client_start(this->mqttClient);
}