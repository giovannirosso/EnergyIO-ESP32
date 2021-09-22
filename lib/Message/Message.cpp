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

//-------------------Serial
Message::Message(uint64_t _datetime, double _data, DataType _type)
{
  this->user = NULL;

  uint8_t buffer[128];
  DataReport msg = DataReport_init_zero;

  pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

  msg.datetime, _datetime;
  msg.data = _data;
  msg.type = _type;
  pb_encode(&stream, DataReport_fields, &msg);

  printf("\nSERIAL confirmation : ");
  for (int i = 0; i < stream.bytes_written; i++)
  {
    data[i] = buffer[i];
    printf("%02X", buffer[i]);
  }
  printf("\n");

  this->length = stream.bytes_written;
}

void Message::r_userData()
{
  DataReport msg DataReport_init_zero;
  pb_istream_t stream = pb_istream_from_buffer(this->dado, this->length);
  pb_decode(&stream, DataReport_fields, &msg);

  printf("datetime :%d", msg.datetime);
  printf("DataType: %d", msg.type);
  printf("Data: %s", msg.data);

  // this->user = msg.user;
}