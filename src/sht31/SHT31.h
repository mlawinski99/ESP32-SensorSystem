#ifndef SHT31SENSOR_H
#define SHT31SENSOR_H

#include <Adafruit_SHT31.h>

class SHT31 {
public:
    SHT31();
    void begin();
    void readData();

    float getTemperature() const;
    float getHumidity() const;

private:
    Adafruit_SHT31 sht;
    float temperature;
    float humidity;
};

#endif