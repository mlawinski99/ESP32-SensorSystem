#ifndef BME280SENSOR_H
#define BME280SENSOR_H

#include <Adafruit_BME280.h>

class BME280 {
public:
    BME280();
    void begin();
    void readData();

    float getTemperature() const;
    float getHumidity() const;
    float getPressure() const;

private:
    Adafruit_BME280 bme;
    float temperature;
    float humidity;
    float pressure;
};

#endif