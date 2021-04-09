#ifndef MQTT_CLIENT_H
#define MQTT_CLIENT_H

#include "esp_event.h"
#include "esp_log.h"

#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "mqtt_client.h"

#include "stdbool.h"
#include <string.h>

#include "freertos/event_groups.h"

#define MQTT_CONNECTED_BIT BIT0
#define MQTT_DISCONNECTED_BIT BIT1
#define MQTT_ERROR_BIT BIT2

typedef void (*onMessageCallback)(const char *topic, const char *data, int length);

void (*_callback)(const char *topic, const char *data, int length);

typedef struct _message
{
    char *data;
    char *topic;
    int length;
    uint8_t qos;
    bool retain;
} message;

typedef enum
{
    MQTT_CONNECTED,
    MQTT_DISCONNECTED,
    MQTT_ERROR,
    MQTT_TIMEOUT
} MqttState;

esp_mqtt_client_handle_t _client;
EventGroupHandle_t mqttEventGroup;
MqttState mqttState;

/*
 * @brief Event handler registered to receive MQTT events
 *
 *  This function is called by the MQTT client event loop.
 *
 * @param handler_args user data registered to the event.
 * @param base Event base for the handler(always MQTT Base in this example).
 * @param event_id The id for the received event.
 * @param event_data The data for the event, esp_mqtt_event_handle_t.
 */
void mqttEventHandler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data);

/**
 * Inicia o mqtt e define o cliente no contexto da biblioteca
 * @param host Endereço do servidor MQTT
 * @param port Porta do servidor MQTT
 * @param clientId Identificador do dispositivo para o servidor
*/
esp_mqtt_client_handle_t mqttInit(const char *host, int port, const char *clientId);

/**
 * Inicia a conexão MQTT com o servidor especificado 
 */
void mqttStart();

/**
 * Encerra a conexão MQTT com o servidor
 */
void mqttDisconnect();

/**
 * Define as credenciais para conexão com servidores protegidos
 * @param username Nome de usuário para a conexão
 * @param password Senha para a conexão
 */
esp_err_t setMqttCredentials(const char *username, const char *password);

void setOnMessageCallback(onMessageCallback callback);

int publish(message *message);

int subscribe(const char *topic, uint8_t qos);

int unsubscribe(const char *topic);

MqttState waitForMqttState();

MqttState getMqttState();

#endif // MQTT_CLIENT_H