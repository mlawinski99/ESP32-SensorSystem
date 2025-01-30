#ifndef SDCARD_H
#define SDCARD_H

#include <SD.h>
#include <SPI.h>
#include <vector>

class SDCard {
public:
    SDCard();
    void begin();
    void saveData(const String &data);
    std::vector<String> readData();
    void removeData(const String &data);

private:
    File myFile;
    SPIClass _sdSPI;
};

#endif