#ifndef PROGRAMRUNNER_H
#define PROGRAMRUNNER_H

#include "sd_card/SDCard.h"
#include "bme280/BME280.h"
#include "sht31/SHT31.h"
#include "pcf8563/PCF8563.h"
#include "netserver/NetServer.h"

class ProgramRunner {
public:
    ProgramRunner();
    void setup();
    void loop();

private:
    BME280 bme;
    SHT31 sht;
    PCF8563 rtc;
    NetServer netServer;
    SDCard sdCard;

    unsigned long lastTime;
    unsigned long timerDelay;
    unsigned long timerDbUpload;
    unsigned long timerDb;

    bool isTimeSet;

    void checkSetup();
};

#endif