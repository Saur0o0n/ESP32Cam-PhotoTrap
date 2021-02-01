/*
** ESP32Cam PhotoTrap
**
** Last update: v0.3 - 2021.01.30 by Adrian (Sauron) Siemieniak
**
** As a board choose Wemos D1 Mini ESP32 (not ESP32 Cam)
**
** This code is based on one created by Rui Santos
** Old project details at https://RandomNerdTutorials.com/esp32-cam-pir-motion-detector-photo-capture/
** 
*/
#include "esp_camera.h"
#include "Arduino.h"
#include "FS.h"                // SD Card ESP32
#include "SD_MMC.h"            // SD Card ESP32
#include "soc/soc.h"           // Disable brownour problems
#include "soc/rtc_cntl_reg.h"  // Disable brownour problems
#include "driver/rtc_io.h"
#include <EEPROM.h>            // read and write from flash memory
#include <DS323x.h>
#include <Update.h>
#include <sys/time.h>
#include <stdio.h>
#include <esp_wifi.h>
#include <esp_bt.h>

// define the number of bytes you want to access
#define EEPROM_SIZE 1

const uint8_t picture_awb_delay = 2000; // delay before taking first photo, this allows sensor to do white ballance and exposure measurements
                                        // If you want picture to be taken as fast as possible, put here 0. If you want to auto white balance and exposition work - use 4000+ (4s +)
                                        // or use sacrificial pictures like in default settings - 1st photo after 2s, second 3s, third 4s. Third one should be the best in terms of AWB/AE
const byte picture_count = 3;           // how many pictures should be takend in each boot cycle
const uint8_t picture_delay = 1000;     // ms delay between taking pictures (does not count time to take picture itself)

// DS3231 stuff
#define RTC_ADDR 0x57
DS323x rtc;
const int I2C_SCL = 15;
const int I2C_SCA = 14;


// Pin definition for CAMERA_MODEL_AI_THINKER
#define PWDN_GPIO_NUM     32
#define RESET_GPIO_NUM    -1
#define XCLK_GPIO_NUM      0
#define SIOD_GPIO_NUM     26
#define SIOC_GPIO_NUM     27
#define Y9_GPIO_NUM       35
#define Y8_GPIO_NUM       34
#define Y7_GPIO_NUM       39
#define Y6_GPIO_NUM       36
#define Y5_GPIO_NUM       21
#define Y4_GPIO_NUM       19
#define Y3_GPIO_NUM       18
#define Y2_GPIO_NUM        5
#define VSYNC_GPIO_NUM    25
#define HREF_GPIO_NUM     23
#define PCLK_GPIO_NUM     22
 
int pictureNumber = 0;

// Check if DS clock is connected
bool isI2CDeviceConnected(const uint8_t addr) {
    Wire.beginTransmission(addr);
    return Wire.endTransmission() == 0;
}


// perform the actual update from a given stream
void performUpdate(Stream &updateSource, size_t updateSize) {
   if (Update.begin(updateSize)) {      
      size_t written = Update.writeStream(updateSource);
      if (written == updateSize) {
         Serial.println("Written : " + String(written) + " successfully");
      }
      else {
         Serial.println("Written only : " + String(written) + "/" + String(updateSize) + ". Retry?");
      }
      if (Update.end()) {
         Serial.println("OTA done!");
         if (Update.isFinished()) {
            Serial.println("Update successfully completed. Rebooting.");
         }
         else {
            Serial.println("Update not finished? Something went wrong!");
         }
      }
      else {
         Serial.println("Error Occurred. Error #: " + String(Update.getError()));
      }

   }
   else
   {
      Serial.println("Not enough space to begin OTA");
   }
}

// check given FS for valid update.bin and perform update if available
void updateFromFS(fs::FS &fs) {
   File updateBin = fs.open("/update.bin");
   if (updateBin) {
      if(updateBin.isDirectory()){
         Serial.println("Error, update.bin is not a file");
         updateBin.close();
         return;
      }

      size_t updateSize = updateBin.size();

      if (updateSize > 0) {
         Serial.println("Try to start update");
         performUpdate(updateBin, updateSize);
      }
      else {
         Serial.println("Error, file is empty");
      }

      updateBin.close();
    
      // whe finished remove the binary from sd card to indicate end of the process
      fs.remove("/update.bin");
   }
   else {
      Serial.println("Could not load update.bin from sd root");
   }
}

// Blink RED led to informa about different tasks
//
void blink_red(uint8_t del=200){
  
  // Blink red diod
  digitalWrite(33, LOW);
  delay(del);
  digitalWrite(33, HIGH);
  delay(del);
}

/*
** Since it's a photo trap, and device should be working all the time without user - try to "heal" the problem with a bit of wait and reboot. In normal circumtances this should be halt.
*/
void rebootEspWithReason(String reason, byte err=4){
    Serial.println(reason);
    delay(1000);
    for(byte a=0; a<err; a++){
        // Red diode on/off to see the error
       blink_red(500);
    }
    Serial.println("Going to light sleep...");
    delay(200);
    esp_sleep_enable_timer_wakeup(10000000); //sleep 10 seconds - cool down
    esp_light_sleep_start();    // we could go to deep sleep here, but ESP.restart is more "robust" for fixing errors, then just deepsleep wakeup
    Serial.println("Light sleep wakeup...");
    delay(200);
    ESP.restart();
}

// Convert compile time to system time 
time_t cvt_date(char const *date, char const *time){
    char s_month[5];
    int year;
    struct tm t;
    static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";
    sscanf(date, "%s %d %d", s_month, &t.tm_mday, &year);
    sscanf(time, "%2d %*c %2d %*c %2d", &t.tm_hour, &t.tm_min, &t.tm_sec);
    // Find where is s_month in month_names. Deduce month value.
    t.tm_mon = (strstr(month_names, s_month) - month_names) / 3 + 1;    
    t.tm_year = year - 1900;    
    return mktime(&t);
}

// Set camera settings, one provided below are (in my taste) most efficient for further processing
//
void set_camera_options(){
  sensor_t * s = esp_camera_sensor_get();
  s->set_brightness(s, 0);     // -2 to 2
  s->set_contrast(s, 0);       // -2 to 2
  s->set_saturation(s, 0);     // -2 to 2
  s->set_special_effect(s, 0); // 0 to 6 (0 - No Effect, 1 - Negative, 2 - Grayscale, 3 - Red Tint, 4 - Green Tint, 5 - Blue Tint, 6 - Sepia)
  s->set_whitebal(s, 1);       // 0 = disable , 1 = enable
  s->set_awb_gain(s, 1);       // 0 = disable , 1 = enable
  s->set_wb_mode(s, 0);        // 0 to 4 - if awb_gain enabled (0 - Auto, 1 - Sunny, 2 - Cloudy, 3 - Office, 4 - Home)
  s->set_exposure_ctrl(s, 1);  // 0 = disable , 1 = enable
  s->set_aec2(s, 1);           // Auto EXP DSP 0 = disable , 1 = enable
  s->set_ae_level(s, 0);       // -2 to 2
  s->set_aec_value(s, 300);    // 0 to 1200
  s->set_gain_ctrl(s, 0);      // 0 = disable , 1 = enable
  s->set_agc_gain(s, 0);       // 0 to 30
  s->set_gainceiling(s, (gainceiling_t)0);  // 0 to 6
  s->set_bpc(s, 0);            // 0 = disable , 1 = enable
  s->set_wpc(s, 1);            // 0 = disable , 1 = enable
  s->set_raw_gma(s, 1);        // 0 = disable , 1 = enable
  s->set_lenc(s, 0);           // 0 = disable , 1 = enable
  s->set_hmirror(s, 0);        // 0 = disable , 1 = enable
  s->set_vflip(s, 1);          // 0 = disable , 1 = enable
  s->set_dcw(s, 0);            // 0 = disable , 1 = enable
  s->set_colorbar(s, 0);       // 0 = disable , 1 = enable
}



void setup() {
char fileName[32];
struct timeval tv;
float temp=-100;

  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); //disable brownout detector
  Serial.begin(115200);

  Serial.setDebugOutput(true);

  // disable wifi/bt
  //esp_wifi_stop();
  //esp_bt_controller_disable();
  
  Wire.begin(I2C_SCA, I2C_SCL);
  
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;

  // Enable red diode
  digitalWrite(33, HIGH); // off
  pinMode(33, OUTPUT);
    
  //pinMode(4, INPUT);
  //digitalWrite(4, LOW);
  //rtc_gpio_hold_dis(GPIO_NUM_4);
 
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA; // FRAMESIZE_ + QVGA|CIF|VGA|SVGA|XGA|SXGA|UXGA
    config.jpeg_quality = 5;
    config.fb_count = 1;  // All examples have 2, but since we are NOT streaming but taking pictures - this is acctualy better (https://github.com/espressif/esp32-camera/blob/master/README.md#important-to-remember)
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 10;
    config.fb_count = 1;
  }
 
  // Init Camera
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    rebootEspWithReason("Camera init failed with error",6);
    return;
  }

  set_camera_options();
    

  rtc.attach(Wire);
  // Time/date stuff - to generate proper filename
  //setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 1);     // Central European Time
  //tzset();

  if(isI2CDeviceConnected(RTC_ADDR)){
    DateTime now = rtc.now();
    tv.tv_sec = now.unixtime()-2092; // Set seconds from epoch in tv structure (don't know why - but my ESP32 is around 35min ahead the clock - tha'ts why - 2092
    Serial.printf("Time from DS3231 is %d:%d:%2.d %d.%d %.2f\n",now.hour(),now.minute(),now.second(),now.day(),now.month(),rtc.temperature());
    temp=rtc.temperature();
    //sprintf(filename,"/%02d-%02d_h%02dm%02ds%02d_%.0fc_%d.jpg",now.month(),now.day(),now.hour(),now.minute(),now.second(),rtc.temperature(),pictureNumber);
  }else{
    //tv.tv_sec = 1611490902; // https://www.epochconverter.com/
    tv.tv_sec = cvt_date(__DATE__, __TIME__)+1500;  // ESP32 was 25min late so I've added it
    Serial.println("Failed to connect to DS3231 - setting time from script compilation time");
  }
  settimeofday(&tv, NULL);    // set the current time of the esp to one from ds or from config

  Serial.println("Starting SD Card...");
  if(!SD_MMC.begin()){
    rebootEspWithReason("Failed to start SD interface",8);
    return;
  }
  uint8_t cardType = SD_MMC.cardType();
  if(cardType == CARD_NONE){
    rebootEspWithReason("Card Mount Failed",9);
    return;
  }
  // Try to do the upgrade - if there is update.bin file
  updateFromFS(SD_MMC);
  
  
  Serial.println("Delay start");
  delay(picture_awb_delay);    // Wait for AWB and AE
  Serial.println("Delay end");

  // initialize EEPROM with predefined size
  EEPROM.begin(EEPROM_SIZE);
  pictureNumber = EEPROM.read(0);


  // Path where new picture will be saved in SD Card
  char fpath[50];
  struct tm timeinfo;
  fs::FS &fs = SD_MMC;
  camera_fb_t *fb = NULL;
  File file;

  for(byte pic=0; pic < picture_count; pic++){
    // Red diode on
    digitalWrite(33, LOW);
    // Take Picture with Camera
    fb = esp_camera_fb_get();

    if(!fb) {
      rebootEspWithReason("Picture capture failed",7);
      return;
    }
    // Disable red diod
    digitalWrite(33, HIGH);

    getLocalTime(&timeinfo);
    strftime(fileName, 20, "/%y-%m-%d_%H.%M.%S", &timeinfo);
    pictureNumber++;
    Serial.printf("Generated filename is: %s\n",fileName);
    if(temp>-50) sprintf(fpath,"%s_%.1f_%d.jpg", fileName, temp, pictureNumber);
    else sprintf(fpath,"%s_%d.jpg", fileName, pictureNumber);

    Serial.printf("Picture file name is: %s\n", fpath);
 
    file = fs.open(fpath, FILE_WRITE);
    if(!file){
      rebootEspWithReason("Failed to open file in writing mode",10);
      return;
    }else {
      Serial.printf("Saved file to path: %s\n", fpath);
      file.write(fb->buf, fb->len); // payload (image), payload length

    }
    file.close();
    esp_camera_fb_return(fb);
  }

  EEPROM.write(0, pictureNumber);
  EEPROM.commit();
    
  // Turns off the ESP32-CAM white on-board LED (flash) connected to GPIO 4
  //pinMode(4, OUTPUT);
  //digitalWrite(4, LOW);
  
  Serial.println("Going to sleep now");
  Serial.flush();
  
  // Blink red diod
  blink_red(150);
  blink_red(150);
  //rtc_gpio_hold_en(GPIO_NUM_4);
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_13, 0);

  //Serial.end();
  delay(500); // this is to flush serial and keep the end part working - do not lower this!

  esp_deep_sleep_start();
} 
 
void loop() {
 
}
