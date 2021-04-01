#include "MqttClient.h"

static const char *TAG = "mqtt client";

void mqttEventHandler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, event_id);
    esp_mqtt_event_handle_t event = event_data;

    switch ((esp_mqtt_event_id_t)event_id)
    {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);

        // message msg;
        // memset(&msg, 0, sizeof(message));

        // strcpy(&msg.data, (const char *)event->data);
        // strcpy(&msg.topic, (const char *)event->topic);
        // msg.length = event->data_len;

        message msg = {
            .data = event->data,
            .topic = event->topic,
            .length = event->data_len,
            .qos = 0,
            .retain = false,
        };

        _callback(&msg);

        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

esp_mqtt_client_handle_t mqttInit(const char *host, int port, const char *clientId)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .host = host,
        .port = port,
        .client_id = clientId,
    };
    // memset(&mqtt_cfg, 0, sizeof(esp_mqtt_client_config_t));

    esp_mqtt_client_handle_t client = esp_mqtt_client_init(&mqtt_cfg);
    /* The last argument may be used to pass data to the event handler, in this example mqtt_event_handler */
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqttEventHandler, NULL);

    _client = client;

    return client;
}

void mqttStart()
{
    esp_mqtt_client_start(_client);
}

void disconnect()
{
    esp_mqtt_client_disconnect(_client);
}

esp_err_t setMqttCredentials(const char *username, const char *password)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .username = username,
        .password = password,
    };

    return esp_mqtt_set_config(_client, &mqtt_cfg);
}

void setOnMessageCallback(onMessageCallback callback)
{
    _callback = callback;
}

int publish(message *message)
{
    return esp_mqtt_client_publish(_client, message->topic, message->data, message->length, message->qos, message->retain);
}

int subscribe(const char *topic, uint8_t qos)
{
    return esp_mqtt_client_subscribe(_client, topic, qos);
}

int unsubscribe(const char *topic)
{
    return esp_mqtt_client_unsubscribe(_client, topic);
}
