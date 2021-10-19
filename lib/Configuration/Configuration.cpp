#include "Configuration.h"

bool Configuration::dirty = false;

char *Configuration::serial = DEVICE_SERIAL;

uint8_t Configuration::data[1024];
uint16_t Configuration::length;

char Configuration::localSsid[MAX_SSID_SIZE] = {DEFAULT_LOCAL_SSID};
char Configuration::localPass[MAX_PASSWORD_SIZE] = {DEFAULT_LOCAL_PASSWORD};
char Configuration::apSsid[MAX_SSID_SIZE] = {DEFAULT_AP_SSID};
char Configuration::apPass[MAX_PASSWORD_SIZE] = {DEFAULT_AP_PASSWORD};

int *Configuration::wifisIntensity = NULL;
String *Configuration::wifisSsids = NULL;
int Configuration::wifisAmount = 0;

float Configuration::v_rms = 0;
float Configuration::i_rms = 0;
int Configuration::pot_ativa = 0;
int Configuration::pot_aparente = 0;
float Configuration::instantMeasure = 0;

char Configuration::sensor[5][7] = {"NODE01", "Node2", "Node3", "Node4", "Node5"};

SensorType Configuration::sensorType[5] = {
    SensorType_ENERGY,
    SensorType_UNDEFINED,
    SensorType_UNDEFINED,
    SensorType_UNDEFINED,
    SensorType_UNDEFINED,
};

int Configuration::lastPipe = NULL;

void Configuration::generateSerial(uint8_t *macAddress)
{
    uint8_t aux1[MAC_SIZE];
    uint8_t aux2[MAC_SIZE];

    for (int i = 0; i < MAC_SIZE; i++)
    {
        aux1[i] = macAddress[i] >> 4;
        aux2[i] = macAddress[i] << 4;
        aux2[i] >>= 4;
    }

    int prefixSize = strlen(DEVICE_TYPE);

    char *_serial = new char[MAC_SIZE + prefixSize + 1];

    strcpy(_serial, DEVICE_TYPE);

    for (int i = 0; i < MAC_SIZE; i++)
    {
        _serial[i + prefixSize + 1] = 0;

        int byteSerial = aux1[i] ^ aux2[(MAC_SIZE - 1) - i];

        if (byteSerial < 10)
        {
            _serial[i + prefixSize] = byteSerial + '0';
        }
        else
        {
            byteSerial -= 10;
            _serial[i + prefixSize] = (char)byteSerial + 'A';
        }
    }

    serial = _serial;
    DPRINTLN(serial);
}

bool Configuration::isDirty()
{
    return dirty;
}

void Configuration::readFlash()
{
    Preferences prefs;

    if (prefs.begin(PREFERENCES_NAMESPACE, true))
    {
        Serial.println("[NVS_FLASH] INIT");
        length = prefs.getBytes("localwifi", data, 1024);
        // readLocalWifi(); //TODO:

        length = prefs.getBytes("apwifi", data, 1024);
        // readApWifi();

        prefs.end();
    }
}

void Configuration::reset()
{
    Preferences prefs;
    DPRINTLN("[NVS_FLASH] Limpando Flash");
    if (prefs.begin(PREFERENCES_NAMESPACE, false))
    {
        prefs.clear();
        prefs.end();
    }
}

///////////////////////////////APwifi//////////////////////////////////////
// void Configuration::setApWifi(char *_apSsid, char *_apPass)
// {
//     byte buffer[128];
//     setWifi_ msg setWifi__init_zero;
//     pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

//     if (_apSsid != NULL)
//     {
//         strcpy(msg.apSsid, _apSsid);
//         strcpy(apSsid, _apSsid);
//     }
//     else
//     {
//         strcpy(msg.apSsid, apSsid);
//     }
//     if (_apPass != NULL)
//     {
//         strcpy(msg.apPass, _apPass);
//         strcpy(apPass, _apPass);
//     }
//     else
//     {
//         strcpy(msg.apPass, apPass);
//     }

//     pb_encode(&stream, setWifi__fields, &msg);

//     Preferences prefs;
//     if (prefs.begin(PREFERENCES_NAMESPACE, false))
//     {
//         prefs.putBytes("apwifi", buffer, stream.bytes_written);
//         prefs.end();
//     }
//     dirty = true;
// }

// void Configuration::readApWifi()
// {
//     if (length != 0)
//     {
//         setWifi_ msg setWifi__init_zero;
//         pb_istream_t stream = pb_istream_from_buffer(data, length);
//         pb_decode(&stream, setWifi__fields, &msg);

//         DPRINTLN("\n[FLASH] Wifi AP Read");
//         DPRINTLN("[FLASH] ApSsid: " + String(msg.apSsid));
//         DPRINTLN("[FLASH] ApPass: " + String(msg.apPass));

//         strcpy(apSsid, msg.apSsid);
//         strcpy(apPass, msg.apPass);
//     }
// }

// ///////////////////////////////LOCAL WIFI//////////////////////////////////
// void Configuration::setLocal(char *_localSsid, char *_localPass)
// {
//     byte buffer[128];
//     setLocal_ msg setLocal__init_zero;
//     pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));

//     if (_localSsid != NULL)
//     {
//         strcpy(msg.localSsid, _localSsid);
//         strcpy(localSsid, _localSsid);
//     }
//     else
//     {
//         strcpy(msg.localSsid, localSsid);
//     }

//     if (_localPass != NULL)
//     {
//         strcpy(msg.localPass, _localPass);
//         strcpy(localPass, _localPass);
//     }
//     else
//     {
//         strcpy(msg.localPass, localPass);
//     }

//     pb_encode(&stream, setLocal__fields, &msg);

//     Preferences prefs;
//     if (prefs.begin(PREFERENCES_NAMESPACE, false))
//     {
//         prefs.putBytes("localwifi", buffer, stream.bytes_written);
//         prefs.end();
//     }
//     dirty = true;
// }

// void Configuration::readLocalWifi()
// {
//     if (length != 0)
//     {
//         setLocal_ msg setLocal__init_zero;
//         pb_istream_t stream = pb_istream_from_buffer(data, length);
//         pb_decode(&stream, setLocal__fields, &msg);

//         DPRINTLN("\n[FLASH] Wifi Local Read");
//         DPRINTLN("[FLASH] LocalSsid: " + String(msg.localSsid));
//         DPRINTLN("[FLASH] LocalPass: " + String(msg.localPass));

//         strcpy(localSsid, msg.localSsid);
//         strcpy(localPass, msg.localPass);
//     }
// }

void Configuration::setWifisScan(int _wifisAmount, int *_wifisIntensity, String *_wifisSsid)
{
    Configuration::wifisAmount = _wifisAmount;
    Configuration::wifisIntensity = new int[_wifisAmount];
    Configuration::wifisSsids = new String[_wifisAmount];

    for (int i = 0; i < _wifisAmount; i++)
    {
        Configuration::wifisIntensity[i] = _wifisIntensity[i];
        Configuration::wifisSsids[i] = _wifisSsid[i];
    }
}

void Configuration::setEnergyReport(float _v_rms, float _i_rms, int _pot_ativa, int _pot_aparente)
{
    v_rms = _v_rms;
    i_rms = _i_rms;
    pot_ativa = _pot_ativa;
    pot_aparente = _pot_aparente;
}

void Configuration::setWaterReport(float _instantMeasure)
{
    instantMeasure = _instantMeasure;
}

void Configuration::setLastPipe(int pipe)
{
    lastPipe = pipe;
}

float Configuration::get_instantMeasure()
{
    return instantMeasure;
}

float Configuration::get_v_rms()
{
    return v_rms;
}

float Configuration::get_i_rms()
{
    return i_rms;
}

int Configuration::get_pot_ativa()
{
    return pot_ativa;
}

int Configuration::get_pot_aparente()
{
    return pot_aparente;
}

char *Configuration::getApSsid()
{
    return Configuration::apSsid;
}

char *Configuration::getLocalSsid()
{
    return Configuration::localSsid;
}

char *Configuration::getApPassword()
{
    return Configuration::apPass;
}

char *Configuration::getLocalPassword()
{
    return Configuration::localPass;
}

char *Configuration::getSerial()
{
    return serial;
}

char *Configuration::getSensorSerial()
{
    if (lastPipe != 0)
        return sensor[lastPipe - 1];
}

SensorType Configuration::getSensorType(int pipe)
{
    return sensorType[pipe - 1];
}

int Configuration::getWifisAmount()
{
    return Configuration::wifisAmount;
}

String *Configuration::getWifisSsid()
{
    return Configuration::wifisSsids;
}

int *Configuration::getWifisIntensity()
{
    return Configuration::wifisIntensity;
}