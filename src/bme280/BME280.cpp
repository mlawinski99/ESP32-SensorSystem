#include "BME280.h"
#include <Wire.h>

BME280::BME280() : temperature(0), humidity(0), pressure(0) {}

void BME280::begin() {
    if (!bme.begin(0x76)) {
        Serial.println("Could not find BME280!");
        while (1);
    }
}

void BME280::readData() {
    temperature = bme.readTemperature();
    humidity = bme.readHumidity();
    pressure = bme.readPressure() / 100.0F;
}

float BME280::getTemperature() const {
    return temperature;
}

float BME280::getHumidity() const {
    return humidity;
}

float BME280::getPressure() const {
    return pressure;
}