#include "SDCard.h"

#define SD_CS 5
#define SD_MOSI 23
#define SD_MISO 19
#define SD_SCK 18

SDCard::SDCard() : _sdSPI(HSPI) {}

void SDCard::begin() {
    _sdSPI.begin(SD_SCK, SD_MISO, SD_MOSI, SD_CS);
    if (!SD.begin(5, _sdSPI)) {
        Serial.println(F("MicroSD Card Mount Failed"));
    }
}

void SDCard::saveData(const String &data) {
    myFile = SD.open("/data.txt", FILE_WRITE);
    if (myFile) {
        size_t c = strlen(data.c_str());
        delay(100);
        size_t result = myFile.write((uint8_t*)data.c_str(), c);
        Serial.print("Wrote ");
        Serial.print(result);
        Serial.println(" bytes");
        delay(100);
        if (c != result)
            Serial.println("Write failed!");
        myFile.close();
        Serial.println(result);
    }
    Serial.println(String(myFile.size()));
}

std::vector<String> SDCard::readData() {
    std::vector<String> savedData;

    if (myFile = SD.open("/data.txt")) {
        Serial.println("Opened txt file!");
        while (myFile.available()) {
            Serial.println("Loop!");
            String body = (myFile.readStringUntil('\n'));
            savedData.push_back(body);
        }
    }
    myFile.close();
    return savedData;
}

void SDCard::removeData(const String &data) {
    String fileName = "/data.txt";
    String buffer;
    String fileName_new = "/data.txt_tmp";

    File myFile_new;
    boolean Removed = false;

    myFile = SD.open(fileName);
    Serial.println(myFile.size());
    myFile_new = SD.open(fileName_new, FILE_WRITE);

    if (myFile_new) {
        Serial.println("Myfilenew is loaded");
        if (myFile) {
            Serial.println("Myfile is loaded");
            while (myFile.available()) {
                buffer = myFile.readStringUntil('\n');
                if (buffer.substring(0, buffer.length() - 1) != data) {
                    myFile_new.println(buffer.substring(0, buffer.length() - 1));
                } else {
                    Removed = true;
                }
            }
            Serial.println(myFile_new.size());
            myFile.close();
            SD.remove(fileName);
        } else {
            Serial.print("error opening ");
            Serial.println(fileName);
        }
        myFile_new.close();
        SD.rename(fileName_new, fileName);
    } else {
        Serial.println("error opening tmp file");
    }
}