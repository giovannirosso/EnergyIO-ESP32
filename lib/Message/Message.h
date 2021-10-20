#ifndef MESSAGE_H
#define MESSAGE_H

#include <Arduino.h>
#include "pb.h"
#include "pb_common.h"
#include "pb_decode.h" // nanopb decode library
#include "pb_encode.h" // nanopb encode library
#include "messages.pb.h"
#include "constants.h"
#include "Configuration.h"

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
	Message(uint8_t *data, uint16_t length);											 // construtor genérico
	Message(char *data, uint16_t length);												 // construtor genéric
	Message(float v_rms, float i_rms, int pot_ativa, int pot_aparente, time_t NTP_time); // construtor energy report
	Message(float instant, time_t NTP_time);
	Message(char _macAddress[32]);
	Message(String _sensorSerial, SensorType _type);
	~Message();
	void print();

	Message(char _serial[10], int32_t _channel);

	void deleteData();
	byte *getMessage();
	uint16_t getLength();
	int getUser();

	void r_EnergyData();
	void r_WaterData();
	void r_pairingMessage();
};

#endif // MESSAGE_H