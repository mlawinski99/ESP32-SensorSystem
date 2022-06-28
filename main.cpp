#include <Wire.h>
#include <SPI.h>
#include "Adafruit_SHT31.h"
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include <WiFi.h>
#include "ESPAsyncWebServer.h"
#include <Rtc_Pcf8563.h>
#include "SD.h"
#include <HTTPClient.h>
#include <time.h>
#define SD_CS 5
#define SD_MOSI  = 23
#define SD_MISO  = 19
#define SD_SCK   = 18

SPIClass _sdSPI(HSPI);

const char* ssid     = "WIFI_SSID";
const char* password = "WIFI_PASSWORD";

const char* serverName = "SERVER_NAME";
String apiKeyValue = "API_KEY";

#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_BME280 bme;
Adafruit_SHT31 sht = Adafruit_SHT31();
Rtc_Pcf8563 rtc;
AsyncWebServer server(80);
AsyncEventSource events("/events");
File myFile;
unsigned long lastTime     = 0;  
unsigned long timerDelay     = 800;//  // read data every 5 seconds
unsigned long timerDbUpload = 1799000; // log data to DB every 30 minutes 1800000 ms ; 3000 ms = 3sec = delay
unsigned long timerDb        = 0;

float temperatureIn;
float humidityIn;
float pressure;

float temperatureOut;
float humidityOut;

bool isTimeSet = false;
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;

void getBME280Data(){
 
  temperatureIn = bme.readTemperature();
  pressure = bme.readPressure() / 100.0F;
  humidityIn = bme.readHumidity();
}
void getSHT31Data()
{
  temperatureOut = sht.readTemperature();
  humidityOut = sht.readHumidity();
}
void setDateTime()
{
  struct tm t;
  if(!getLocalTime(&t))
  {
  Serial.println("Failed to obtain time");
  }
  else
  {
  Serial.println(&t, "%Y %m %d %H:%M:%S");

  byte year = byte(t.tm_year-100);
  byte  month = byte(t.tm_mon+1);
  byte wday = byte(t.tm_wday);
  byte  day = byte(t.tm_mday);
  byte hour = (byte(t.tm_hour));
  byte minute = (byte(t.tm_min));
  byte second = (byte(t.tm_sec));
    rtc.setDateTime(day,wday, month,0, year,  hour, minute, second);
  }
}
String processor(const String& var){
  getBME280Data();
  getSHT31Data();
  if(var == "TEMPERATUREIN"){
    return String(temperatureIn);
  }
  else if(var == "HUMIDITYIN"){
    return String(humidityIn);
  }
  else if(var == "TEMPERATUREOUT"){
    return String(temperatureOut);
  }
  else if(var == "HUMIDITYOUT"){
    return String(humidityOut);
  }
  else if(var == "PRESSURE"){
    return String(pressure);
  }
}

void saveDataSD(String POSTstring)
{
  myFile = SD.open("/data.txt", FILE_WRITE);
  if(myFile)
  {
    size_t c = strlen(POSTstring.c_str());
    delay(100);
    size_t result = myFile.write((uint8_t*)POSTstring.c_str(),c);
    Serial.print("Wrote ");
    Serial.print(result);
    Serial.println(" bytes");
    delay(100);
    if(c!=result)
      Serial.println("Write failed!");
    myFile.close();
   Serial.println(result);
  }
  Serial.println(String(myFile.size()));
}

bool sendData(String post)
{ 
   WiFiClient client;
    HTTPClient http;
    http.begin(client, serverName);
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    int httpResponseCode = http.POST(post);

    if (httpResponseCode>0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String response = http.getString();
      Serial.println(response);
      if(httpResponseCode == 200)
      {
      http.end();
      return true;
      }
    }
    else {
      Serial.print("Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
    timerDb = 0;
    return false;
}
String getRTCData()
{
    rtc.readStatus2();
     String result = "20"+String(rtc.getYear())+"."+String(rtc.getMonth())+"."+String(rtc.getDay())+" "+String(rtc.getHour())+":"+String(rtc.getMinute())+":00";
  
    Serial.println(result);
    return  result;
}
void removeData(String str)
{
    String fileName = "/data.txt";
    String buffer;
    String fileName_new = "/data.txt_tmp";

    File myFile;
    File myFile_new;
    boolean Removed = false;
    myFile = SD.open(fileName);
    Serial.println(myFile.size());
    myFile_new = SD.open(fileName_new, FILE_WRITE);

    if (myFile_new) 
    {
        Serial.println("Myfilenew is loaded");
        if (myFile) 
        {
            Serial.println("Myfile is loaded");
            while (myFile.available()) 
            {
                  buffer = myFile.readStringUntil('\n');
                if (buffer.substring(0, buffer.length() - 1) != str) 
                {
                    myFile_new.println(buffer.substring(0, buffer.length() - 1));
                } 
                else {
                    Removed = true;
                }
            }
            Serial.println(myFile_new.size());
            myFile.close();
            SD.remove(fileName);

        } 
        else 
        {
            Serial.print("error opening ");
            Serial.println(fileName);
        }
        myFile_new.close();
        SD.rename(fileName_new, fileName);
    } 
    else 
    {
        Serial.println("error opening tmp file");
    }
}

void readDataSD()
{
  String POSTstring;
  if(myFile = SD.open("/data.txt"))
  {
    Serial.println("Opened txt file!");
     while (myFile.available()) {
    Serial.println("Loop!");
    POSTstring = (myFile.readStringUntil('\n'));
    if(sendData(POSTstring)==true)
        removeData(POSTstring);
    }
  }
  myFile.close();
}


void checkSetup()
{
if (!bme.begin(0x76)) {
    Serial.println("Could not find BME280!");
    while (1);
     }
    if (! sht.begin(0x44))
    {
     Serial.println("Could not find SHT31!");
      while (1) delay(1);
    }

_sdSPI.begin(18, 19, 23, 5); //CLK,MISO,MOSI,CS
  if (!SD.begin(SD_CS, _sdSPI)) {
    Serial.println(F("MicroSD Card Mount Failed"));
  }
}

void setup() {
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
  if(WiFi.status() == WL_CONNECTED)
  {
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  isTimeSet = true;
  }
  checkSetup();
  rtc.initClock();
  setDateTime();
  // server
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processor);
  });

  //events
  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // reconnect delay 1 second
    client->send("hello!", NULL, millis(), 10000);
  });
  server.addHandler(&events);
  server.begin();
}

void loop() {
 
 if ((millis() - lastTime) > timerDelay) {
   
    getBME280Data();
    getSHT31Data();
    Serial.printf("Temperature = %.2f ºC \n", temperatureIn);
    Serial.printf("Humidity = %.2f % \n", humidityIn);
    Serial.printf("Pressure = %.2f hPa \n", pressure);
    Serial.printf("TemperatureOut = %.2f ºC \n", temperatureOut);
    Serial.printf("HumidityOut = %.2f % \n", humidityOut);
    Serial.println();

    String time = getRTCData();
    // Send Events to the Web Server with the Sensor Data
    events.send("ping",NULL,millis());
    events.send(String(temperatureIn).c_str(),"temperatureIn",millis());
    events.send(String(humidityIn).c_str(),"humidityIn",millis());
    events.send(String(temperatureOut).c_str(),"temperatureOut",millis());
    events.send(String(humidityOut).c_str(),"humidityOut",millis());
    events.send(String(pressure).c_str(),"pressure",millis());

    timerDb += millis() - lastTime;
    Serial.println("TimerDb: "+ String(timerDb));
    lastTime = millis();

    if((int(rtc.getMinute()) == 0 || int(rtc.getMinute()) == 30) && int(rtc.getSecond()) == 0)
    { time=getRTCData();
      float avgTempIn = 0;
      float avgTempOut = 0;
      float avgHumIn = 0; 
      float avgHumOut = 0; 
      float avgPressure = 0;
      if(isTimeSet == true)
      {
        for(int i=0;i<3;i++)
      {
        getBME280Data();
        getSHT31Data();
        avgTempIn += temperatureIn;
        avgTempOut += temperatureOut;
        avgHumIn += humidityIn; 
        avgHumOut += humidityOut;
        avgPressure += pressure;
        delay(1000);
      }
        String httpRequestData = "api_key=" + apiKeyValue + "&tempIn=" + String((avgTempIn/3))+ "&humidityIn=" + String((avgHumIn/3)).c_str() + "&tempOut=" + String((avgTempOut/3)).c_str()+ "&humidityOut=" + String((avgHumOut/3)).c_str() + "&pressure=" + String((avgPressure/3)).c_str() + "&time="+time;
        saveDataSD(httpRequestData);
        readDataSD();
        avgTempIn = 0;
        avgTempOut = 0;
        avgHumIn = 0;
        avgHumOut = 0;
        avgPressure = 0;
      }
      else
      {
        if(WiFi.status() == WL_CONNECTED)
        {
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
        setDateTime();
        isTimeSet = true;
        }
      }
    }
}
}
  
