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
        xEventGroupSetBits(mqttEventGroup, MQTT_CONNECTED_BIT);
        mqttState = MQTT_CONNECTED;
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        xEventGroupSetBits(mqttEventGroup, MQTT_DISCONNECTED_BIT);
        mqttState = MQTT_DISCONNECTED;
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
        printf("DATA LEN=%d\r\n", event->data_len);

        char *topic = (char *)malloc(sizeof(char) * event->topic_len + 1);
        memset(topic, 0, event->topic_len + 1);
        memcpy(topic, event->topic, event->topic_len);

        char *data = (char *)malloc(sizeof(char) * event->data_len + 1);
        memset(data, 0, event->data_len + 1);
        memcpy(data, event->data, event->data_len);

        _callback((const char *)topic, (const char *)data, event->data_len);

        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        xEventGroupSetBits(mqttEventGroup, MQTT_ERROR_BIT);
        mqttState = MQTT_ERROR;
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

    ESP_LOGI("mqtt", "before event group init");

    mqttEventGroup = xEventGroupCreate();

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

MqttState waitForMqttState()
{
    vPortYield();
    EventBits_t bits = xEventGroupWaitBits(mqttEventGroup,
                                           MQTT_CONNECTED_BIT | MQTT_DISCONNECTED_BIT | MQTT_ERROR_BIT,
                                           pdTRUE,
                                           pdFALSE,
                                           portMAX_DELAY);

    if (bits & MQTT_CONNECTED_BIT)
    {
        return MQTT_CONNECTED;
    }
    else if (bits & MQTT_DISCONNECTED_BIT)
    {
        return MQTT_DISCONNECTED;
    }
    else if (bits & MQTT_ERROR_BIT)
    {
        return MQTT_ERROR;
    }
    else
    {
        return MQTT_TIMEOUT;
    }
}

MqttState getMqttState()
{
    return mqttState;
}