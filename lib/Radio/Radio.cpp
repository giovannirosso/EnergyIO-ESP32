#include "constants.h"
#include "WString.h"
#include "Radio.h"

float payload = 0.0;

uint8_t node_serial[][6] = {"1Node", "2Node"};

RADIO::RADIO()
{
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
    nrf24.setPayloadSize(sizeof(payload));
    // nrf24.setCRCLength();
    // nrf24.setChannel(CHANNEL);

    nrf24.openWritingPipe(node_serial[0]);
    nrf24.openReadingPipe(1, node_serial[1]);
    // nrf24.openReadingPipe(2, node_serial[2]);

    nrf24.startListening();
    nrf24.printPrettyDetails();
}

void RADIO::changeRole(bool role)
{
    if (role)
    {
        nrf24.stopListening(); // put radio in TX mode
    }
    else
    {
        nrf24.startListening(); // put radio in RX mode
    }
}

void RADIO::listen()
{
    uint8_t pipe;
    if (nrf24.available(&pipe))
    {                                           // is there a payload? get the pipe number that recieved it
        uint8_t bytes = nrf24.getPayloadSize(); // get the size of the payload
        nrf24.read(&payload, bytes);            // fetch payload from FIFO
        Serial.print(F("Received "));
        Serial.print(bytes); // print the size of the payload
        Serial.print(F(" bytes on pipe "));
        Serial.print(pipe); // print the pipe number
        Serial.print(F(": "));
        Serial.println(payload); // print the payload's value
    }
}