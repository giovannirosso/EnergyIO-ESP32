#include "Radio.h"

uint8_t data[32];
uint16_t length;

uint8_t RADIO::node_address[6][6] = {"Hub00", "Node1", "Node2", "Node3", "Node4", "Node5"};

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

    nrf24.openWritingPipe(node_address[0]);
    nrf24.openReadingPipe(1, node_address[1]);
    // nrf24.openReadingPipe(2, node_address[2]);
    // nrf24.openReadingPipe(3, node_address[3]);
    // nrf24.openReadingPipe(4, node_address[4]);
    // nrf24.openReadingPipe(5, node_address[5]);

    changeRole(nrfRole);
    nrf24.printPrettyDetails();
}

void RADIO::changeRole(bool role)
{
    if (role)
    {
        nrf24.startListening(); // put radio in RX mode //! true = RX
        DPRINTLN("startListening");
    }
    else
    {
        nrf24.stopListening(); // put radio in TX mode  //! false = TX
        DPRINTLN("stopListening");
    }
    DPRINTLN("Role changed");
    nrfRole = role;
    delay(500);
}

void RADIO::listen()
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

        //? if(pipe == ENERGY SENSOR TYPE)
        income.r_EnergyData();
        //? elseif(pipe == WATER SENSOR TYPE)
        // income.r_WaterData();
    }
}

void RADIO::report()
{
    int j = 0;
    for (int i = 0; i < 32; i++)
    {
        data[i] = j;
        if (j == 9)
            j = 0;
        j++;
    }

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
    char *Serial = Configuration::getSerial();

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