#ifndef MESSAGE_H
#define MESSAGE_H

#include <Arduino.h>
#include "pb.h"
#include "pb_common.h"
#include "pb_decode.h" // nanopb decode library
#include "pb_encode.h" // nanopb encode library
#include "messages.pb.h"

#include "WString.h"

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
	Message(char *data, uint16_t length);	 // construtor genéric
	~Message();

	Message(uint64_t _datetime, double _data, DataType _type);

	void deleteData();
	byte *getMessage();
	uint16_t getLength();
	int getUser();

	void r_userData(); //construtor confirmation
};

#endif //MESSAGE_H