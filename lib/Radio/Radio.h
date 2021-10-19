#ifndef RADIO_H
#define RADIO_H

#include "Configuration.h"
#include "constants.h"
#include "WString.h"
#include "Control.h"
#include "RF24.h"
#include "Message.h"
#include <SPI.h>
// #include "printf.h"

#define RECEIVER true
// #define TRANSMITTER

#define CHANNEL 69 // 127

class RADIO
{
private:
    RF24 nrf24;
    bool nrfRole;
    static uint8_t node_address[6][6];

public:
    RADIO();
    ~RADIO();
    void init();
    void changeRole(bool role);
    int listen();
    void report();
    bool getRole();
    bool pairingMode();
    bool listenPairing();
};

#endif // RADIO_H