#include "Local.h"

Local::Local(String ssid, String password)
{

    IPAddress localIp(192, 168, 4, 1);
    IPAddress subnetMask(255, 255, 255, 0);

    if (!WiFi.softAPConfig(localIp, localIp, subnetMask))
    {
        DPRINTF("Failed to configure SoftAP");
        return;
    }
    vTaskDelay(500);

    if (!WiFi.softAP(ssid.c_str(), password.c_str()))
    {
        DPRINT("Failed to initialize SoftAP");
        return;
    }
    vTaskDelay(500);

    this->server = new WebServer();

    this->server->on('/local', HTTP_POST, [this]()
                     { wifiRequestHandler(); });
    this->server->on('/hub', HTTP_POST, [this]()
                     { wifiRequestHandler(); });
    this->server->on('/save', HTTP_POST, [this]()
                     { saveRequestHandler(); });
    this->server->on('/wifis', HTTP_POST, [this]()
                     { wifiListRequestHandler(); });

    this->server->serveStatic("/", SPIFFS, "/index.html");

    this->server->enableCORS();
    this->server->begin(SERVER_PORT);
}

void Local::wifiRequestHandler()
{

    DPRINTF("[LOCAL] New request");
    DPRINTF("Path: %s\n", this->server->uri());

    if (this->server->method() != HTTP_POST)
    {
        this->server->send(400, "text/html", "Invalid method");
        return;
    }

    if (!this->server->hasArg("ssid"))
    {
        this->server->send(400, "text/html", "Invalid body");
        return;
    }

    if (!this->server->hasArg("password"))
    {
        this->server->send(400, "text/html", "Invalid body");
        return;
    }

    String ssid = this->server->arg('ssid');
    String password = this->server->arg('password');

    if (this->server->uri() == "/local")
    {
        Configuration::setLocal(ssid, password);
    }
    else if (this->server->uri() == "/ap")
    {
        Configuration::setApWifi(ssid, password);
    }

    this->server->send(200, "text/html", "ok");
}

void Local::saveRequestHandler()
{
    if (this->server->method() != HTTP_POST)
    {
        this->server->send(400, "text/html", "Invalid method");
        return;
    }

    if (this->server->hasArg("save"))
    {
        this->server->send(200, "text/html", "ok");
        ESP.restart();
    }
}

void Local::wifiListRequestHandler()
{
    if (this->server->method() != HTTP_GET)
    {
        this->server->send(400, "text/html", "Invalid method");
        return;
    }

    Serial.println("NEW WIFI LIST REQUEST");

    int wifiLength = WiFi.scanNetworks();

    String wifis = "[";

    for (uint8_t i = 0; i < wifiLength; i++)
    {
        wifis += "{"
                 "\"ssid\":\"" +
                 WiFi.SSID(i) + "\"," + "\"rssi\":" + String(WiFi.RSSI(i)) + "}";

        if (i != wifiLength - 1)
        {
            wifis += ",";
        }
    }

    wifis += "]";

    this->server->send(200, "application/x-www-form-urlencoded", wifis);
}