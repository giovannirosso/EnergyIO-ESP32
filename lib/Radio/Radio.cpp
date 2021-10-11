#include "constants.h"
#include "WString.h"
#include "Radio.h"

float payload = 0.0;

uint8_t data[32];
uint16_t length;

uint8_t node_serial[][6] = {"1Node", "2Node"};

RADIO::RADIO()
{
    nrfRole = false;
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

    nrf24.openWritingPipe(node_serial[0]);
    nrf24.openReadingPipe(1, node_serial[1]);
    // nrf24.openReadingPipe(2, node_serial[0]);

    changeRole(nrfRole);
    nrf24.printPrettyDetails();
}

void RADIO::changeRole(bool role)
{
    if (role)
    {
        nrf24.stopListening(); // put radio in TX mode  //! TRUE = TX
    }
    else
    {
        nrf24.startListening(); // put radio in RX mode //! false = RX
    }
    DPRINTLN("Role changed");
    nrfRole = role;
}

void RADIO::listen()
{
    uint8_t pipe;
    if (nrf24.available(&pipe))
    {                                           // is there a payload? get the pipe number that recieved it
        uint8_t bytes = nrf24.getPayloadSize(); // get the size of the payload
        nrf24.read(&data, bytes);               // fetch payload from FIFO
        Serial.print(F("Received "));
        Serial.print(bytes); // print the size of the payload
        Serial.print(F(" bytes on pipe "));
        Serial.print(pipe); // print the pipe number
        Serial.print(F(": "));
        // Serial.println(payload); // print the payload's value
        for (int i = 0; i < bytes; i++)
        {
            Serial.printf("%02X", data[i]);
        }
        Serial.printf("\n");
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