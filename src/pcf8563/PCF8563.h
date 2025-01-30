#ifndef PCF8563_H
#define PCF8563_H

#include <Rtc_Pcf8563.h>

class PCF8563 {
public:
    PCF8563();
    void begin();
    void setDateTime();
    String getDateTime();

private:
    Rtc_Pcf8563 rtc;
};

#endif
