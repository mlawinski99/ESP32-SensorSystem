#include "PCF8563.h"
#include <time.h>

PCF8563::PCF8563() {}

void PCF8563::begin() {
    rtc.initClock();
}

void PCF8563::setDateTime() {
    struct tm t;
    if (!getLocalTime(&t)) {
        Serial.println("Failed to obtain time");
    } else {
        byte year = byte(t.tm_year - 100);
        byte month = byte(t.tm_mon + 1);
        byte wday = byte(t.tm_wday);
        byte day = byte(t.tm_mday);
        byte hour = byte(t.tm_hour);
        byte minute = byte(t.tm_min);
        byte second = byte(t.tm_sec);
        rtc.setDateTime(day, wday, month, 0, year, hour, minute, second);
    }
}

String PCF8563::getDateTime() {
    rtc.readStatus2();
    String result = "20" + String(rtc.getYear()) + "." + String(rtc.getMonth()) + "." + String(rtc.getDay()) + " " + String(rtc.getHour()) + ":" + String(rtc.getMinute()) + ":00";
    Serial.println(result);
    return result;
}