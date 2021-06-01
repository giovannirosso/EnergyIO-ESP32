#ifndef PROTOMESSAGE_H
#define PROTOMESSAGE_H

#include "pb.h"
#include "pb_common.h"
#include "pb_decode.h" // nanopb decode library
#include "pb_encode.h" // nanopb encode library
#include "message.pb.h"

#include <stdio.h>

#define MAX_SSID_SIZE 128

#define MIN_PASSWORD_SIZE 8
#define MAX_PASSWORD_SIZE 32

class Message
{
    private:
    uint8_t *dado;
	uint8_t data[128];
	uint16_t length;
	int user;

    public:
    Message(uint8_t *data, uint16_t length); // construtor genérico
    ~Message();

    Message(int _user, DataType _type, char *_data);																//construtor confirmation
};

#endif //PROTOMESSAGE_H