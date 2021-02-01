#include "Arduino.h"
#include <DS323x.h>
#include <sys/time.h>
#include <WiFi.h>
#include <EEPROM.h>            // read and write from flash memory

// define the number of bytes you want to access
#define EEPROM_SIZE 1
const int pictureNumber=0;

// DS3231 stuff
#define RTC_ADDR 0x57
DS323x rtc;
const int I2C_SCL = 15;
const int I2C_SCA = 14;

// WiFi details
const char *ssid     = "***";
const char *password = "***";

// NTP
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 3600;
const int   daylightOffset_sec = 3600;



// Check if DS clock is connected
bool isI2CDeviceConnected(const uint8_t addr) {
    Wire.beginTransmission(addr);
    return Wire.endTransmission() == 0;
}


void WiFi_Setup(){

  Serial.printf("Connecting to %s ",ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected!");
}


void setup() {
char datetime[40];

  Serial.begin(115200);
  Serial.println("Starting...");
  Wire.begin(I2C_SCA, I2C_SCL);
  WiFi_Setup();
  struct tm timeinfo;
  getLocalTime(&timeinfo);
  strftime(datetime, 40, "%Y-%m-%d %H:%M:%S", &timeinfo);
  Serial.printf("Current date on ESP32 is: %s\n",datetime);
  Serial.println(" | syncing NTP servers...");
  // Start NTP Time Client
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  Serial.println(" | setting internal clock from NTP");
  struct timeval tv;
  
  getLocalTime(&timeinfo);
  strftime(datetime, 40, "%Y-%m-%d %H:%M:%S", &timeinfo);
  Serial.printf("Current date on ESP32 is: %s\n",datetime);

  if(!isI2CDeviceConnected(RTC_ADDR)){
    Serial.println(" *** DS3231 not connected - exiting *** ");
    return;
  }
  Serial.println(" | connecting to DS3231...");
  rtc.attach(Wire);
//  DateTime now = rtc.now();
  sprintf(datetime,"%02d-%02d-%02d %02d:%02d:%02d %.2fC",rtc.year(),rtc.month(),rtc.day(),rtc.hour(),rtc.minute(),rtc.second(),rtc.temperature());
  Serial.printf("Current date on DS3231 is: %s\n",datetime);

  Serial.println(" | setting NTP time on DS3231...");
//  getLocalTime(&timeinfo);
//  rtc.now(DateTime(timeinfo.tm_year, timeinfo.tm_mon, timeinfo.tm_mday, timeinfo.tm_hour, timeinfo.tm_min, timeinfo.tm_sec));
  rtc.now(DateTime(time(NULL)+gmtOffset_sec));

  Serial.println(" | getting new time from DS3231...");
  sprintf(datetime,"%02d-%02d-%02d %02d:%02d:%02d %.2fC",rtc.year(),rtc.month(),rtc.day(),rtc.hour(),rtc.minute(),rtc.second(),rtc.temperature());
  Serial.printf("Current date on DS3231 is: %s\n",datetime);
  Serial.println("Clock setup finished!");

  Serial.printf(" | resetting EEPROM picture count to %d\n",pictureNumber);
  EEPROM.begin(EEPROM_SIZE);
  EEPROM.write(0, pictureNumber);
  EEPROM.commit();
  Serial.printf("Clock setup finished!\n");
}

void loop() {
  // put your main code here, to run repeatedly:

}
