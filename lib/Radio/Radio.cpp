#include "Radio.h"

uint8_t data[32];
uint16_t length;

// uint8_t RADIO::node_address[6][6] = {"Hub00", "EA101", "Node2", "Node3", "Node4", "Node5"};

RADIO::RADIO()
{
    nrfRole = true;
}

RADIO::~RADIO()
{
}

void RADIO::init()
{
    if (!nrf24.begin(4, 5))
    {
        Serial.println(F("radio hardware is not responding!!"));
        while (1)
        {
        }
    }
    nrf24.setPALevel(RF24_PA_MAX);
    nrf24.setDataRate(RF24_250KBPS);
    // nrf24.setDataRate(RF24_1MBPS);
    nrf24.setCRCLength(RF24_CRC_16);
    nrf24.enableDynamicPayloads();
    nrf24.enableAckPayload();
    nrf24.setPayloadSize(32);
    // nrf24.setChannel(CHANNEL);
    uint8_t hubSerial[5];
    uint8_t sensor[5][6];
    for (int i = 0; i < 5; i++)
    {
        sensor[0][i] = Configuration::getSensor()[0][i];
        sensor[1][i] = Configuration::getSensor()[1][i];
        sensor[2][i] = Configuration::getSensor()[2][i];
        sensor[3][i] = Configuration::getSensor()[3][i];
        sensor[4][i] = Configuration::getSensor()[4][i];
    }

    uint8_t hub[6] = "HUB01";
    nrf24.openWritingPipe(hub);
    nrf24.openReadingPipe(1, sensor[0]);
    nrf24.openReadingPipe(2, sensor[1]);
    nrf24.openReadingPipe(3, sensor[2]);
    nrf24.openReadingPipe(4, sensor[3]);
    nrf24.openReadingPipe(5, sensor[4]);

    changeRole(nrfRole);
    nrf24.printPrettyDetails();
}

void RADIO::changeRole(bool role)
{
    if (role)
    {
        nrf24.startListening(); // put radio in RX mode //! true = RX
        Serial.println("startListening");
    }
    else
    {
        nrf24.stopListening(); // put radio in TX mode  //! false = TX
        Serial.println("stopListening");
    }
    Serial.println("Role changed");
    nrfRole = role;
    delay(500);
}

int RADIO::listen()
{
    uint8_t pipe;
    if (nrf24.available(&pipe))
    {
        Control::led2(!Control::led2());        // is there a payload? get the pipe number that recieved it
        uint8_t bytes = nrf24.getPayloadSize(); // get the size of the payload
        nrf24.read(&data, bytes);               // fetch payload from FIFO
        Serial.print(F("Received "));
        Serial.print(bytes); // print the size of the payload
        Serial.print(F(" bytes on pipe "));
        Serial.print(pipe); // print the pipe number
        Serial.print(F(": "));
        // Serial.println(payload); // print the payload's value
        uint8_t msg_length = data[31];
        uint8_t msg_buffer[msg_length];
        for (int i = 0; i < msg_length; i++)
        {
            Serial.printf("%02X", data[i]);
            msg_buffer[i] = data[i];
        }
        Serial.printf("\n");
        Message income(msg_buffer, msg_length);

        if (Configuration::getSensorType(pipe) == SensorType_WATER)
        {
            income.r_WaterData();
            Configuration::setLastPipe(pipe);
            return 1;
        }
        else if (Configuration::getSensorType(pipe) == SensorType_ENERGY)
        {
            income.r_EnergyData();
            Configuration::setLastPipe(pipe);
            return 2;
        }
        else
        {
            Serial.println("[SENSOR] NOT FOUND");
            return 0;
        }
    }
    return 0;
}

void RADIO::report()
{
    unsigned long start_time = millis();              //
    bool reported = nrf24.write(&data, sizeof(data)); // transmit & save the report
    unsigned long end_time = millis();

    if (reported)
    {
        printf("Transmission successful!"); // payload was delivered
        printf("Tranmission time %lu ms\nSent: ", end_time - start_time);
        for (int i = 0; i < length; i++)
        {
            printf("%02X", data[i]);
        }
        printf("\n");
    }
    else
    {
        printf("\nTransmission fail!\n");
    }
}

bool RADIO::getRole()
{
    return nrfRole;
}

bool RADIO::pairingMode()
{
    int sensorNumber = Configuration::getTotalSensors();
    char Serial[5];
    for (int i = 0; i < 5; i++)
    {
        Serial[i] = Configuration::getSensor()[4 - sensorNumber][i];
        printf("%c", Serial[i]);
    }

    Message message(Serial, CHANNEL);

    unsigned long start_time = millis();                   //
    bool reported = nrf24.write(message.getMessage(), 32); // transmit & save the report
    unsigned long end_time = millis();

    uint8_t *aux = message.getMessage();
    if (reported)
    {
        printf("\nPairing successful!"); // payload was delivered
        printf("\nPairing time %lu ms\nSent: ", end_time - start_time);
        for (int i = 0; i < 32; i++)
        {
            printf("%d", aux[i]);
        }
        printf("\n");
        changeRole(true);
    }
    else
        printf("\nPairing fail!\n");
    return reported;
}

bool RADIO::listenPairing()
{
    uint8_t pipe;
    if (nrf24.available(&pipe))
    {
        Control::led2(!Control::led2());        // is there a payload? get the pipe number that recieved it
        uint8_t bytes = nrf24.getPayloadSize(); // get the size of the payload
        nrf24.read(&data, bytes);               // fetch payload from FIFO
        Serial.print(F("Received "));
        Serial.print(bytes); // print the size of the payload
        Serial.print(F(" bytes on pipe "));
        Serial.print(pipe); // print the pipe number
        Serial.print(F(": "));
        // Serial.println(payload); // print the payload's value
        uint8_t msg_length = data[31];
        uint8_t msg_buffer[msg_length];
        for (int i = 0; i < msg_length; i++)
        {
            Serial.printf("%02X", data[i]);
            msg_buffer[i] = data[i];
        }
        Serial.printf("\n");
        Message income(msg_buffer, msg_length);

        income.r_pairingMessage();
        return true;
    }
    return false;
}