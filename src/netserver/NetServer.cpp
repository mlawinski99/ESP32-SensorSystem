#include <SPIFFS.h>
#include "NetServer.h"

NetServer::NetServer() : server(80), events("/events"), bme280(), sht31() {}

void NetServer::readIndexHtml() {
    File file = SPIFFS.open("/index.html", "r");
    if (!file) {
        Serial.println("Failed to open index.html");
        return;
    }
    index_html = file.readString();
    file.close();
}

void NetServer::begin() {
    if (!SPIFFS.begin()) {
        Serial.println("Failed to mount file system");
        return;
    }
    readIndexHtml();
    server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request){
        String response = processResponse(request->arg("var"));
        request->send(200, "text/html", response);
    });
    events.onConnect([](AsyncEventSourceClient *client) {
        if (client->lastId()) {
            Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
        }
        client->send("hello!", NULL, millis(), 10000);
    });
    server.addHandler(&events);
    server.begin();
}

bool NetServer::sendData(String body)
{
    http.begin(client, serverName);
    http.addHeader("Content-Type", "application/json");

    int httpResponseCode = http.POST(body);
    if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String response = http.getString();
        Serial.println(response);
        if(httpResponseCode == 200)
        {
            http.end();
            return true;
        }
    }
    else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
    }
    http.end();
    return false;
}

void NetServer::sendData(const String &temperatureIn, const String &humidityIn, const String &temperatureOut, const String &humidityOut, const String &pressure) {
    events.send("ping", NULL, millis());
    events.send(temperatureIn.c_str(), "temperatureIn", millis());
    events.send(humidityIn.c_str(), "humidityIn", millis());
    events.send(temperatureOut.c_str(), "temperatureOut", millis());
    events.send(humidityOut.c_str(), "humidityOut", millis());
    events.send(pressure.c_str(), "pressure", millis());
}

String NetServer::processResponse(const String& var) {
	bme280.readData();
	sht31.readData();

    if (var == "TEMPERATUREIN") {
        return String(bme280.getTemperature());
    } else if (var == "HUMIDITYIN") {
        return String(bme280.getHumidity());
    } else if (var == "TEMPERATUREOUT") {
        return String(sht31.getTemperature());
    } else if (var == "HUMIDITYOUT") {
        return String(sht31.getHumidity());
    } else if (var == "PRESSURE") {
        return String(bme280.getPressure());
    }
    return String();
}