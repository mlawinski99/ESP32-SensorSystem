#include "SHT31.h"
#include <Wire.h>

SHT31::SHT31() : temperature(0), humidity(0) {}

void SHT31::begin() {
    if (!sht.begin(0x44)) {
        Serial.println("Could not find SHT31!");
        while (1) delay(1);
    }
}

void SHT31::readData() {
    temperature = sht.readTemperature();
    humidity = sht.readHumidity();
}

float SHT31::getTemperature() const {
    return temperature;
}

float SHT31::getHumidity() const {
    return humidity;
}