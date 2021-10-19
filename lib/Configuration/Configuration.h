#ifndef CONFIGURATION_H
#define CONFIGURATION_H

#include <Arduino.h>
#include <Preferences.h>
#include <constants.h>
#include "pb.h"
#include "pb_common.h"
#include "pb_decode.h" // nanopb decode library
#include "pb_encode.h" // nanopb encode library
#include "messages.pb.h"
#include <WString.h>

#define PREFERENCES_NAMESPACE (const char *)"HUB-config"

#define DEVICE_SERIAL "XX1234ABC"
#define DEVICE_TYPE "MA"
#define MAC_SIZE 6
#define DEFAULT_AP_SSID "EnergyIO HUB"
#define DEFAULT_AP_PASSWORD "12345678"
#define DEFAULT_USER 0

#define DEFAULT_LOCAL_SSID "Rosso"
#define DEFAULT_LOCAL_PASSWORD "055A64F7"

#define MAX_SSID_SIZE 128
#define MAX_PASSWORD_SIZE 32

class Configuration
{
private:
    static uint8_t data[1024];
    static uint16_t length;

    static char localSsid[MAX_SSID_SIZE];
    static char localPass[MAX_PASSWORD_SIZE];
    static char apSsid[MAX_SSID_SIZE];
    static char apPass[MAX_PASSWORD_SIZE];

    static int wifisAmount;
    static int *wifisIntensity;
    static String *wifisSsids;

    static float v_rms;
    static float i_rms;
    static int pot_ativa;
    static int pot_aparente;
    static float instantMeasure;

    static char *serial;
    static char sensor[5][7];
    static int lastPipe;
    static SensorType sensorType[5];
    static bool dirty;

public:
    static bool isDirty();
    static void readFlash();
    static void reset();

    static void setLastPipe(int pipe);
    static void setApWifi(char *_apSsid, char *_apPass);
    static void setLocal(char *_localSsid, char *_localPass);
    static void setWifisScan(int _wifisAmount, int *_wifisIntensity, String *wifisSsid);
    static void setEnergyReport(float _v_rms, float _i_rms, int _pot_ativa, int _pot_aparente);
    static void setWaterReport(float _instant);

    static void readLocalWifi();
    static void readApWifi();

    static char *getApSsid();
    static char *getLocalSsid();
    static char *getApPassword();
    static char *getLocalPassword();
    static int getWifisAmount();
    static int *getWifisIntensity();
    static String *getWifisSsid();
    static float get_v_rms();
    static float get_i_rms();
    static int get_pot_ativa();
    static int get_pot_aparente();
    static float get_instantMeasure();

    static void generateSerial(uint8_t *macAddress);
    static char *getSerial();
    static char *getSensorSerial();
    static SensorType getSensorType(int pipe);
};

#endif // CONFIGURATION_H
