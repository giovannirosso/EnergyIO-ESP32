#ifndef LOCAL_H
#define LOCAL_H

#include "WebServer.h"
#include "WiFi.h"
#include "constants.h"
#include "SPIFFS.h"
#include "Configuration.h"

class Local
{
private:
    WebServer *server;

    void wifiRequestHandler();
    void saveRequestHandler();
    void wifiListRequestHandler();

public:
    Local(String ssid, String password);

    void handle();
};

#endif //LOCAL_H