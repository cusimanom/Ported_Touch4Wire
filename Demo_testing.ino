#include "Touch_4Wire.h"

#define YP A1  // must be an analog pin, use "An" notation!
#define XM A0  // must be an analog pin, use "An" notation!
#define YM D1  // can be a digital pin
#define XP D0  // can be a digital pin


TouchScreen ts = TouchScreen(XP, YP, XM, YM, 326);

void setup(void) {
  Serial.begin(9600);
  delay(5000);
  Serial.println("Ready");
  //int rawY = ts.readTouchY();
}

void loop(void) {
    
  TSPoint p = ts.getPoint();
 ts.readTouchY();
  
  
  
  //Serial.print("raw X = "); Serial.print(rawX);
  //Serial.print("\traw Y = "); Serial.println(rawY);

  if(p.z > ts.pressureThreshhold){
     Serial.print("X = "); Serial.print(p.x);
     Serial.print("\tY = "); Serial.print(p.y);
     Serial.print("\tPressure = "); Serial.println(p.z);
 }

  delay(100);
}