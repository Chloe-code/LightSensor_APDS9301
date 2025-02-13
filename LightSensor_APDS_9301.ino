#include "Wire.h"
#include <Sparkfun_APDS9301_Library.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>

#ifndef STASSID
#define STASSID "********"   //Wifi SSID
#define STAPSK  "********"   //Wifi Password
#endif

APDS9301 apds;

//#define INT_PIN 2 // We'll connect the INT pin from our sensor to the
                  // INT0 interrupt pin on the Arduino.
bool lightIntHappened = false; // flag set in the interrupt to let the
                  //  mainline code know that an interrupt occurred.

void setup() 
{
  Serial.begin(115200);
  WiFi.begin(STASSID, STAPSK); 
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Wire.begin();

  // APDS9301 sensor setup.
  apds.begin(0x39);  // We're assuming you haven't changed the I2C
                     //  address from the default by soldering the
                     //  jumper on the back of the board.
  apds.setGain(APDS9301::LOW_GAIN); // Set the gain to low. Strictly
                     //  speaking, this isn't necessary, as the gain
                     //  defaults to low.
  apds.setIntegrationTime(APDS9301::INT_TIME_13_7_MS); // Set the
                     //  integration time to the shortest interval.
                     //  Again, not strictly necessary, as this is
                     //  the default.
  apds.setLowThreshold(0); // Sets the low threshold to 0, effectively
                     //  disabling the low side interrupt.
  apds.setHighThreshold(50); // Sets the high threshold to 500. This
                     //  is an arbitrary number I pulled out of thin
                     //  air for purposes of the example. When the CH0
                     //  reading exceeds this level, an interrupt will
                     //  be issued on the INT pin.
  apds.setCyclesForInterrupt(1); // A single reading in the threshold
                     //  range will cause an interrupt to trigger.
  apds.enableInterrupt(APDS9301::INT_ON); // Enable the interrupt.
  apds.clearIntFlag();

  // Interrupt setup
//  pinMode(INT_PIN, INPUT_PULLUP); // This pin must be a pullup or have
                     //  a pullup resistor on it as the interrupt is a
                     //  negative going open-collector type output.
  attachInterrupt(digitalPinToInterrupt(2), lightInt, FALLING);
}

void loop() 
{
  if (WiFi.status() == WL_CONNECTED)
  {
    static unsigned long outLoopTimer = 0;
    apds.clearIntFlag();                          

    // This is a once-per-second timer that calculates and prints off
    //  the current lux reading.
    if (millis() - outLoopTimer >= 1000)
    {
      outLoopTimer = millis();

      Serial.print("Luminous flux: ");
      Serial.println(apds.readLuxLevel(),6);
      String lux = String(apds.readLuxLevel(),6);
      
      if (lightIntHappened)
      {
        Serial.println("Interrupt");
        lightIntHappened = false;
      }     
      HTTPClient http;
      http.begin("http://***.***.**.**/WebService/WebService.asmx/insert_CDs");  //Webservice URL
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");                
      String a1=String("CDs=");          
      int httpCode = http.POST(a1+lux);   
      String payload = http.getString();  
      Serial.println(a1+lux);
      Serial.println(httpCode);            
      Serial.println(payload);             
      http.end();
      delay(60000);
    }    
  }
}

void lightInt()
{
  lightIntHappened = true;
}
