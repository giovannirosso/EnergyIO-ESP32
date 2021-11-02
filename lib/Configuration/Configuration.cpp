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

int Configuration::totalSensors = 0;

String Configuration::sensorSerials[5] = {"NODE1", "NODE2", "NODE3", "NODE4", "NODE5"};
String Configuration::sensor[5] = {"1NODE", "2NODE", "3NODE", "4NODE", "5NODE"};

String Configuration::lastRegistered = "";

SensorType Configuration::sensorType[5] = {
    SensorType_UNDEFINED,
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
    Serial.println(serial);
}

bool Configuration::isDirty()
{
    return dirty;
}

void Configuration::readFlash()
{
    Preferences prefs;

    readApWifi();
    readLocalWifi();

    if (prefs.begin(SENSORS_PROPRETIES, true))
    {
        length = prefs.getBytes("sensor", data, 1024);
        if (length != 0)
        {
            SensorToFlash msg = SensorToFlash_init_zero;
            pb_istream_t stream = pb_istream_from_buffer(data, length);
            pb_decode(&stream, SensorToFlash_fields, &msg);

            Serial.println("\n[FLASH] SetRF Read");
            totalSensors = msg.totalSensors;
            Serial.println("[FLASH] Numero de Sensores: " + String(msg.totalSensors));
            for (int i = 4; i >= 0; i--)
            {
                Serial.println("[FLASH] Sensor Serial " + String(i + 1) + " : " + String(msg.sensorSerial[i]));
                Serial.println("[FLASH] Tipo " + String(i + 1) + " : " + String(msg.sensorType[i]));

                sensorSerials[i] = msg.sensorSerial[i];
                sensorType[i] = msg.sensorType[i];
            }
        }

        prefs.end();
    }
}

void Configuration::reset()
{
    Preferences prefs;
    Serial.println("[NVS_FLASH] Limpando Flash");
    if (prefs.begin(PREFERENCES_NAMESPACE, false))
    {
        prefs.clear();
        prefs.end();
    }
    if (prefs.begin(SENSORS_PROPRETIES, false))
    {
        prefs.clear();
        prefs.end();
    }
}

///////////////////////////// APwifi //////////////////////////////////////
void Configuration::setApWifi(String _apSsid, String _apPass)
{
    Preferences prefs;

    if (prefs.begin(PREFERENCES_NAMESPACE))
    {
        prefs.putString("/ap/ssid", _apSsid);
        prefs.putString("/ap/password", _apPass);
        prefs.end();
    }
}

void Configuration::readApWifi()
{
    Preferences prefs;

    if (prefs.begin(PREFERENCES_NAMESPACE))
    {
        String ssid = prefs.getString("/ap/ssid", DEFAULT_AP_SSID);
        String password = prefs.getString("/ap/password", DEFAULT_AP_PASSWORD);

        DPRINTF("[NVS_FLASH] AP SSID: %s\n", ssid.c_str());
        DPRINTF("[NVS_FLASH] AP PASS: %s\n", password.c_str());

        strcpy(apSsid, ssid.c_str());
        strcpy(apPass, password.c_str());
        prefs.end();
    }
}

// ///////////////////////////////LOCAL WIFI//////////////////////////////////
void Configuration::setLocal(String _apSsid, String _apPass)
{
    Preferences prefs;

    if (prefs.begin(PREFERENCES_NAMESPACE))
    {
        prefs.putString("/local/ssid", _apSsid);
        prefs.putString("/local/password", _apPass);
        prefs.end();
    }
}

void Configuration::readLocalWifi()
{
    Preferences prefs;

    if (prefs.begin(PREFERENCES_NAMESPACE))
    {
        String ssid = prefs.getString("/local/ssid", DEFAULT_AP_SSID);
        String password = prefs.getString("/local/password", DEFAULT_AP_PASSWORD);

        DPRINTF("[NVS_FLASH] LOCAL SSID: %s\n", ssid.c_str());
        DPRINTF("[NVS_FLASH] LOCAL PASS: %s\n", password.c_str());

        strcpy(localSsid, ssid.c_str());
        strcpy(localPass, password.c_str());
        prefs.end();
    }
}

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

void Configuration::setSensor(char *_sensorSerial, SensorType _type)
{
    Serial.println("1");
    byte buffer[512];
    SensorToFlash msg SensorToFlash_init_zero;
    pb_ostream_t stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
    Serial.println("2");
    if (Configuration::totalSensors >= 5)
    {
        Serial.println("MAX SENSORS REACHED");
        return;
    }
    Serial.println("3");
    sensorSerials[4 - totalSensors] = _sensorSerial;
    Serial.println("4");
    lastRegistered = _sensorSerial;
    Serial.println("5");
    sensorType[4 - totalSensors] = _type;
    Serial.println("6");
    Configuration::totalSensors++;

    DPRINTF("[FLASH] Numero de sensores %d\n", Configuration::totalSensors);

    for (int i = 0; i < 5; i++) // escreve os controles ja cadastrados
    {
        strcpy(msg.sensorSerial[i], sensorSerials[i].c_str());
        msg.sensorType[i] = sensorType[i];
        DPRINTF("SENSOR NUMBER %d\t msg.sensorSerial[%d] = %s\t msg.sensorType[%d] = %d\n", i + 1, i, msg.sensorSerial[i], i, msg.sensorType[i]);
    }

    msg.totalSensors = totalSensors;

    pb_encode(&stream, SensorToFlash_fields, &msg);
    DPRINTF("[FLASH] Bytes escritos: %d", stream.bytes_written);
    Serial.println();

    Preferences prefs;
    if (prefs.begin(SENSORS_PROPRETIES, false))
    {
        prefs.putBytes("sensor", buffer, stream.bytes_written);
        prefs.end();
    }
    dirty = true;
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

String Configuration::getLastRegistered()
{
    return lastRegistered;
}

String Configuration::getSensorInPipeSerial()
{
    if (lastPipe != 0)
        return sensorSerials[lastPipe - 1];
}

String *Configuration::getSensorSerial()
{
    return sensorSerials;
}

String *Configuration::getSensor()
{
    return sensor;
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

int Configuration::getTotalSensors()
{
    return Configuration::totalSensors;
}