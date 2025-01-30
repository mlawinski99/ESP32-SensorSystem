#ifndef NETSERVER_H
#define NETSERVER_H

#include <ESPAsyncWebServer.h>
#include <bme280/BME280.h>
#include <sht31/SHT31.h>
#include <env.h>
#include <HTTPClient.h>

class NetServer {
public:
    NetServer();
    void begin();
    void sendData(const String &temperatureIn, const String &humidityIn, const String &temperatureOut, const String &humidityOut, const String &pressure);
    bool sendData(String post);

private:
    AsyncWebServer server;
    AsyncEventSource events;
    String index_html;
    BME280 bme280;
    SHT31 sht31;
    WiFiClient client;
    HTTPClient http;

    void readIndexHtml();
    String processResponse(const String& var);
};

#endif