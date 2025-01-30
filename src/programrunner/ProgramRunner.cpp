#include "ProgramRunner.h"
#include <Wire.h>
#include <SPI.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <env.h>

ProgramRunner::ProgramRunner()
    : lastTime(0), timerDelay(800), timerDbUpload(59900), timerDb(0), isTimeSet(false) {}

void ProgramRunner::checkSetup() {
    bme.begin();
    sht.begin();
    sdCard.begin();
}

void ProgramRunner::setup() {
    Serial.begin(115200);
    Wire.begin(21, 22);

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(1000);
        Serial.println("Setting as a Wi-Fi Station..");
    }
    Serial.print("Station IP Address: ");
    Serial.println(WiFi.localIP());
    Serial.println();

    if (WiFi.status() == WL_CONNECTED) {
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
        isTimeSet = true;
    }

    checkSetup();
    rtc.begin();
    rtc.setDateTime();

    netServer.begin();
}

void ProgramRunner::loop() {
    if (millis() - lastTime > timerDelay) {
        bme.readData();
        sht.readData();
        Serial.printf("Temperature = %.2f ºC \n", bme.getTemperature());
        Serial.printf("Humidity = %.2f % \n", bme.getHumidity());
        Serial.printf("Pressure = %.2f hPa \n", bme.getPressure());
        Serial.printf("TemperatureOut = %.2f ºC \n", sht.getTemperature());
        Serial.printf("HumidityOut = %.2f % \n", sht.getHumidity());
        Serial.println();

        String time = rtc.getDateTime();
        netServer.sendData(
            String(bme.getTemperature(), 2),
            String(bme.getHumidity(), 2),
            String(sht.getTemperature(), 2),
            String(sht.getHumidity(), 2),
            String(bme.getPressure(), 2)
        );

        timerDb += millis() - lastTime;
        Serial.println("TimerDb: " + String(timerDb));
        lastTime = millis();

        if (timerDb >= timerDbUpload) {
            time = rtc.getDateTime();
            float avgTempIn = 0;
            float avgTempOut = 0;
            float avgHumIn = 0;
            float avgHumOut = 0;
            float avgPressure = 0;

            if (isTimeSet) {
                for (int i = 0; i < 3; i++) {
                    bme.readData();
                    sht.readData();
                    avgTempIn += bme.getTemperature();
                    avgTempOut += sht.getTemperature();
                    avgHumIn += bme.getHumidity();
                    avgHumOut += sht.getHumidity();
                    avgPressure += bme.getPressure();
                    delay(1000);
                }
                String requestBody = "{";
                requestBody += "\"tempIn\":" + String(avgTempIn / 3, 2) + ",";
                requestBody += "\"humidityIn\":" + String(avgHumIn / 3, 2) + ",";
                requestBody += "\"tempOut\":" + String(avgTempOut / 3, 2) + ",";
                requestBody += "\"humidityOut\":" + String(avgHumOut / 3, 2) + ",";
                requestBody += "\"pressure\":" + String(avgPressure / 3, 2) + ",";
                requestBody += "\"time\":\"" + time + "\"";
                requestBody += "}";

                sdCard.saveData(requestBody);
                std::vector<String> savedData = sdCard.readData(); // return postString

                for (size_t i = 0; i < savedData.size(); i++) {
                    bool sentSuccessfully = netServer.sendData(savedData[i]); // send to function
                    if (sentSuccessfully) {
                        sdCard.removeData(savedData[i]);
                    }
                }
                timerDb = 0;

                avgTempIn = 0;
                avgTempOut = 0;
                avgHumIn = 0;
                avgHumOut = 0;
                avgPressure = 0;
            } else {
                if (WiFi.status() == WL_CONNECTED) {
                    configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
                    rtc.setDateTime();
                    isTimeSet = true;
                }
            }
        }
    }
}