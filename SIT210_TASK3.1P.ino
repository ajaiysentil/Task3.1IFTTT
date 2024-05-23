#include <WiFiNINA.h>
#include "secrets.h"
#include <BH1750.h> 
#include <Wire.h> 

// Pin connections for Arduino Uno:
// VCC -> 5V (or 3.3V)
// GND -> Ground
// SDA -> A4
// SCL -> A5

BH1750 lightMeter; 


//please enter your sensitive data in the Secret tab
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASSWORD;

WiFiClient client;

char   HOST_NAME[] = "maker.ifttt.com";
String PATH_NAME = "/trigger/IFTTT trigger light sensor/with/key/bK8KiH8YCjnLFGSJfURUmX"; // change your EVENT-NAME and YOUR-KEY
String queryString = "?value1=57&value2=25"; 

bool aboveLimit = false; 
int limit = 300;
unsigned long lightExposureBegining = 0; 
unsigned long lightExposureTime = 0;
unsigned long lastResetTime = 0; 
unsigned long currentTime; 



void setup() {
  // initialize WiFi connection
  WiFi.begin(ssid, pass);

  Serial.begin(9600);
  while (!Serial);

  Wire.begin(); 
  lightMeter.begin(); 

  if (client.connect(HOST_NAME, 80)) {
    Serial.println("Connected to server");
  }
  else {
    Serial.println("connection failed");
  }

  unsigned totalTime = millis(); 
}
void sendNotification(String event, float lux) {
  String queryString = "?value1=" + event + "&value2=" + String(lux) + " lx";


  client.println("GET " + PATH_NAME + queryString + " HTTP/1.1");
  client.println("Host: " + String(HOST_NAME));
  client.println("Connection: close");
  client.println();

  // Wserver response 
  while (client.connected()) {
    if (client.available()) {
      char c = client.read();
      Serial.print(c);
    }
  }

  client.stop();
  Serial.println();
  Serial.println("Disconnected");
}



void loop() {

  currentTime = millis(); 

   if (currentTime - lastResetTime >= 86400000) {
    lightExposureTime = 0;
    lastResetTime = currentTime;
  }

  float lux = lightMeter.readLightLevel();

  if (lux < 0){
    Serial.println("Light sensor failure");
  }
  else{
  Serial.print("Light level is: ");
  Serial.print(lux);
  Serial.println(" lx");
  //send a notification if sunlight has occured and passed limit 
  if (lux > limit && !aboveLimit) {
    lightExposureBegining = currentTime; 
    sendNotification("Sunlight detected", lux);
    aboveLimit = true;

  } else if (lux <= limit && aboveLimit) {
    unsigned long lightExposureEnd = currentTime; // Log the end time of the exposure
    lightExposureTime += (lightExposureEnd - lightExposureBegining) / 1000; // Update total exposure time in seconds
    sendNotification("Sunlight stopped", lux);
    aboveLimit = false;
  }

   //notify the light exposure at the end of each day 
    if (lightExposureTime >= 7200) { // 7200 seconds = 2 hours
      sendNotification("Daily sunlight exposure", lightExposureTime / 60);
      lightExposureTime = 0; // reset light exposure time as the day is complete 

  }

  
  delay(10000); // check every 10 sec 
}



}