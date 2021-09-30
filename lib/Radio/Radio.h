#ifndef RADIO_H
#define RADIO_H

#include "constants.h"
#include "RF24.h"
#include <SPI.h>
// #include "printf.h"

#define RECEIVER true
// #define TRANSMITTER

#define CHANNEL 69 //127

class RADIO
{
private:
    RF24 nrf24;

public:
    RADIO();
    ~RADIO();
    void init();
    void changeRole(bool role);
    void listen();
};

#endif //RADIO_H