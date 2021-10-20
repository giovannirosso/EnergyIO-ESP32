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

Message::Message(float v_rms, float i_rms, int pot_ativa, int pot_aparente, time_t NTP_time)
{
  this->user = NULL;

  uint8_t buffer[128];
  EnergyReport msg = EnergyReport_init_zero;

  pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

  msg.v_rms = v_rms;
  msg.i_rms = i_rms;
  msg.pot_ativa = pot_ativa;
  msg.pot_aparente = pot_aparente;
  msg.datetime = NTP_time;
  pb_encode(&stream, EnergyReport_fields, &msg);

  printf("\nEnergyReport Msg : ");
  for (int i = 0; i < stream.bytes_written; i++)
  {
    data[i] = buffer[i];
    printf("%02X", buffer[i]);
  }
  printf("\n");

  this->length = stream.bytes_written;
}

Message::Message(float instant, time_t NTP_time)
{
  this->user = NULL;

  uint8_t buffer[128];
  WaterReport msg = WaterReport_init_zero;

  pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

  msg.instant = instant;
  msg.datetime = NTP_time;
  pb_encode(&stream, WaterReport_fields, &msg);

  printf("\nWaterReport Msg : ");
  for (int i = 0; i < stream.bytes_written; i++)
  {
    data[i] = buffer[i];
    printf("%02X", buffer[i]);
  }
  printf("\n");

  this->length = stream.bytes_written;
}

Message::Message(char _macAddress[32])
{
  this->user = NULL;

  uint8_t buffer[128];
  HubRegister msg = HubRegister_init_zero;

  pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

  strcpy(msg.mac_address, _macAddress);
  strcpy(msg.secret, "segredo");
  pb_encode(&stream, HubRegister_fields, &msg);

  printf("\nHubRegister Msg : ");
  for (int i = 0; i < stream.bytes_written; i++)
  {
    data[i] = buffer[i];
    printf("%02X", buffer[i]);
  }
  printf("\n");

  this->length = stream.bytes_written;
}

Message::Message(String _sensorSerial, SensorType _type)
{
  this->user = NULL;

  uint8_t buffer[128];
  SensorRegister msg = SensorRegister_init_zero;

  pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

  strcpy(msg.sensor_serial, _sensorSerial.c_str());
  msg.type = _type;
  pb_encode(&stream, SensorRegister_fields, &msg);

  printf("\nSensorRegister Msg : ");
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
         "Pot.Ativa:%d - Pot Aparente:%d - Numero de Msgs:%d\r\n",
         msg.v_rms, msg.i_rms, msg.pot_ativa, msg.pot_aparente, msg.samples);

  if (msg.samples == 0)
    Configuration::setEnergyReport(msg.v_rms, msg.i_rms, msg.pot_ativa, msg.pot_aparente);
  else
    Configuration::setEnergyReport(msg.v_rms / msg.samples, msg.i_rms / msg.samples, msg.pot_ativa / msg.samples, msg.pot_aparente / msg.samples);
}

void Message::r_WaterData()
{
  WaterSensorReport msg = WaterSensorReport_init_zero;
  pb_istream_t stream = pb_istream_from_buffer(this->dado, this->length);
  pb_decode(&stream, WaterSensorReport_fields, &msg);

  printf("DECODED: Numero de Msgs: %d	Instantaneo:%.1f mL/seg\r\n", msg.samples, msg.instant);

  if (msg.samples == 0)
    Configuration::setWaterReport(msg.instant);
  else
    Configuration::setWaterReport(msg.instant / msg.samples);
}

void Message::r_pairingMessage()
{
  PairingMessage msg = PairingMessage_init_zero;
  pb_istream_t stream = pb_istream_from_buffer(this->dado, this->length);
  pb_decode(&stream, PairingMessage_fields, &msg);

  printf("DECODED: Serial: %s	Channel: %d\r\n", msg.serial, msg.channel);

  SensorType type;

  if (msg.serial[0] == 'W')
    type = SensorType_WATER;
  else if (msg.serial[0] == 'E')
    type = SensorType_ENERGY;
  else
  {
    DPRINTF("UNDEFINED SENSOR");
    return;
  }

  Configuration::setSensor(msg.serial, type);
}