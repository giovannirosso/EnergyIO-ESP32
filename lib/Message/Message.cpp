#include "Message.h"

// ---------------------------------------CONTRUTOR GENÉRICO
Message::Message(uint8_t *_data, uint16_t _length)
{
  this->dado = _data;
  this->length = _length;

  this->user = NULL;
}

Message::Message(char *_data, uint16_t _length)
{
  this->dado = (uint8_t *)_data;
  this->length = _length;

  this->user = NULL;
}

void Message::print()
{
  DPRINTF("\n[MSG] DATA   : ");
  for (int i = 0; i < length; i++)
  {
    DPRINTF("%02X", data[i]);
  }
  DPRINTLN();
}

Message::~Message()
{
  // delete[] this->
}

void Message::deleteData()
{
  delete[] data;
}

byte *Message::getMessage()
{
  return this->data;
}

uint16_t Message::getLength()
{
  return this->length;
}

int Message::getUser()
{
  return this->user;
}

Message::Message(char _serial[10], int32_t _channel)
{
  this->user = NULL;

  uint8_t buffer[128];
  PairingMessage msg = PairingMessage_init_zero;

  pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

  strcpy(msg.serial, _serial);
  msg.channel = _channel;
  pb_encode(&stream, PairingMessage_fields, &msg);

  printf("\nPairing Msg : ");
  for (int i = 0; i < stream.bytes_written; i++)
  {
    data[i] = buffer[i];
    printf("%02X", buffer[i]);
  }
  printf("\n");

  this->length = stream.bytes_written;
}

void Message::r_EnergyData()
{
  EnergySensorReport msg = EnergySensorReport_init_zero;
  pb_istream_t stream = pb_istream_from_buffer(this->dado, this->length);
  pb_decode(&stream, EnergySensorReport_fields, &msg);

  printf("DECODED: Tensao: %.1f - Corrente:%.3f - "
         "Pot.Ativa:%d - Pot Aparente:%d - Numero de Amostas:%d\r\n",
         msg.v_rms, msg.i_rms, msg.pot_ativa, msg.pot_aparente, msg.samples);

  // this->user = msg.user;
}

void Message::r_WaterData()
{
  WaterSensorReport msg = WaterSensorReport_init_zero;
  pb_istream_t stream = pb_istream_from_buffer(this->dado, this->length);
  pb_decode(&stream, WaterSensorReport_fields, &msg);

  printf("DECODED: Amostras: %d	Instantaneo:%.1f mL/seg\r\n", msg.samples, msg.instant);
}

void Message::r_pairingMessage()
{
  PairingMessage msg = PairingMessage_init_zero;
  pb_istream_t stream = pb_istream_from_buffer(this->dado, this->length);
  pb_decode(&stream, PairingMessage_fields, &msg);

  printf("DECODED: Serial: %s	Channel: %d\r\n", msg.serial, msg.channel);
}