#include <Ticker.h>
#include <Arduino.h>
#include "WiFi.h"

Ticker ticker;

//WiFi Credentials
const char* ssid     = "";
const char* password = "";

#define Threshold 10 /* Greater the value, more the sensitivity */
#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  60        /* Time ESP32 will go to sleep (in seconds) */

#define WIFI_TIMEOUT  30        /* Time ESP32 will go to sleep (in seconds) */

touch_pad_t touchPin;
bool InitWifiDone = false;

RTC_DATA_ATTR int bootCount = 0; //If each time the device can't connect to WiFi, bootCount will incremented
int maxBootCount = 10; //If bootCount reaches certain times, device will always sleep (no timer)

void callback(){
  //placeholder callback function
}

void InitWifi(){
  //Intial Wifi
  ticker.attach(WIFI_TIMEOUT,sleepWifi);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("Connecting to WiFi..");
    }
  Serial.println("Connected to the WiFi network");
  Serial.println("IP address set: ");
  Serial.println(WiFi.localIP()); //print LAN IP
  InitWifiDone = true;
  ticker.detach();

}

void sleepWifi(){
  ++bootCount;
  setSleep(TIME_TO_SLEEP);
}

int setSleep(int sleepTime){
  //Set Sleep
  //Print the wakeup reason for ESP32 and touchpad too
  esp_sleep_enable_timer_wakeup(sleepTime * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(sleepTime) +
  " Seconds");
  
  //Setup interrupt on Touch Pad 3 (GPIO15)
  touchAttachInterrupt(T3, callback, Threshold);
  //Configure Touchpad as wakeup source
  esp_sleep_enable_touchpad_wakeup();

  //Go to sleep now
  Serial.println("Going to sleep now");
  delay(1000);
  esp_deep_sleep_start();
  Serial.println("This will never be printed");
}

void setup() {
  Serial.begin(115200);
  // wait until serial port opens for native USB devices
  while (! Serial) {
    delay(1);
  }
  Serial.println("Boot number: " + String(bootCount));
  
  if(bootCount > maxBootCount){
    Serial.println("Device has scan WiFi more than (times): " + String(bootCount));
    Serial.println("Device will always in deep sleep"); //Device will deep sleep forever to preserve the battery
    
    //Setup interrupt on Touch Pad 3 (GPIO15)
    touchAttachInterrupt(T3, callback, Threshold);
    
    //Configure Touchpad as wakeup source
    esp_sleep_enable_touchpad_wakeup();
    esp_deep_sleep_start();
  }

  InitWifi();
  bootCount = 0; //Reset the bootCount if WiFi finally connected.
  Serial.print("Device has connected to WiFi before timeout!");
  // put your setup code here, to run once:
  
}

void loop() {
}
