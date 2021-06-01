#include "ProtoMessage.h"

// ---------------------------------------CONTRUTOR GENÉRICO
Message::Message(uint8_t *_data, uint16_t _length)
{
  this->dado = _data;
  this->length = _length;

  this->user = NULL;
}

Message::~Message()
{
  // delete[] this->
}

//-------------------Serial
Message::Message(int _user, DataType _type, char *_data)
{
  this->user = NULL;

  uint8_t buffer[128];
  userData msg = userData_init_zero;

  pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

  strcpy(msg.data, _data);
  msg.user = _user;
  msg.type = _type;
  pb_encode(&stream, userData_fields, &msg);

  printf("\nSERIAL confirmation : ");
  for (int i = 0; i < stream.bytes_written; i++)
  {
    data[i] = buffer[i];
    printf("%02X", buffer[i]);
  }
  printf("\n");

  this->length = stream.bytes_written;
}