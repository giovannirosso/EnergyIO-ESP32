#ifndef RADIO_H
#define RADIO_H

#include "constants.h"
#include "RF24.h"

#define RECEIVER true
// #define TRANSMITTER

#define CHANNEL 1 //127

class RADIO
{
private:
    bool connected;

public:
    RADIO(RF24 &nrfClient);
    ~RADIO();
};

#endif //RADIO_H