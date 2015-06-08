// Touch screen library with X Y and Z (pressure) readings as well
// as oversampling to avoid 'bouncing'
// (c) ladyada / adafruit
// Code under MIT License
//
// ported for Spark Core by ScruffR Nov. 2014 additions Wesner0019 June 2015
// https://github.com/ScruffR/touch_4wire/tree/master/firmware

#include "Touch_4Wire.h"
#include "application.h"
#if  (PLATFORM_ID) == 6
   STM32_Pin_Info* PIN_MAP = HAL_Pin_Map(); // Pointer required for highest access speed
#endif

// We found 2 is precise yet not too slow so we suggest sticking with it!
// increase or decrease the touchscreen oversampling. This is a little different than you may think:
// 1 is no oversampling, whatever data we get is immediately returned
// 2 is double-sampling and we only return valid data if both points are the same
// 3+ uses insert sort to get the median value.
// We found 2 is precise yet not too slow so we suggest sticking with it!

#define NUMSAMPLES 2

TouchScreen::TouchScreen(uint8_t xp, uint8_t yp, uint8_t xm, uint8_t ym,
       uint16_t rxplate) {
  _yp = yp;
  _xm = xm;
  _ym = ym;
  _xp = xp;
  _rxplate = rxplate;
  pressureThreshhold = 10;
}

TSPoint::TSPoint(void) {
  x = y = 0;
}

TSPoint::TSPoint(int16_t x0, int16_t y0, int16_t z0) {
  x = x0;
  y = y0;
  z = z0;
}

bool TSPoint::operator==(TSPoint p1) {
  return  ((p1.x == x) && (p1.y == y) && (p1.z == z));
}

bool TSPoint::operator!=(TSPoint p1) {
  return  ((p1.x != x) || (p1.y != y) || (p1.z != z));
}

#if (NUMSAMPLES > 2)
static void insert_sort(int array[], uint8_t size) {
  uint8_t j;
  int save;

  for (int i = 1; i < size; i++) {
    save = array[i];
    for (j = i; j >= 1 && save < array[j - 1]; j--)
      array[j] = array[j - 1];
    array[j] = save;
  }
}
#endif

TSPoint TouchScreen::getPoint(void) {
  int x, y, z;
  int samples[NUMSAMPLES];
  uint8_t i, valid;

  valid = 1;

  pinMode(_yp, INPUT);
  pinMode(_ym, INPUT);

  // ScruffR: I guess this should detach PULL_UPs

  // ScruffR: Since Spark Core has no PULL UPs with INPUT - not needed
  pinSetLow(_yp);
  pinSetLow(_ym);


  pinMode(_xp, OUTPUT);
  pinMode(_xm, OUTPUT);


  pinSetHigh(_xp);
  pinSetLow(_xm);


   for (i=0; i<NUMSAMPLES; i++) {
     samples[i] = analogRead(_yp);
   }

#if NUMSAMPLES > 2
   insert_sort(samples, NUMSAMPLES);
#endif
#if NUMSAMPLES == 2
  if (abs(samples[0] - samples[1]) > XY_TOLERANCE) { valid = 0; }
#endif

   x = (ADC_MAX_VALUE - samples[NUMSAMPLES/2]);

   pinMode(_xp, INPUT);
   pinMode(_xm, INPUT);

   // ScruffR: I guess this should detach PULL_UPs

   // ScruffR: Since Spark Core has no PULL UPs with INPUT - not needed
   pinSetLow(_xp);


   pinMode(_yp, OUTPUT);
   pinMode(_ym, OUTPUT);


   pinSetHigh(_yp);
   pinSetLow(_ym);  // ScruffR: since we didn't do it before



   for (i=0; i<NUMSAMPLES; i++) {
     samples[i] = analogRead(_xm);
   }

#if NUMSAMPLES > 2
   insert_sort(samples, NUMSAMPLES);
#endif
#if NUMSAMPLES == 2
   if (abs(samples[0] - samples[1]) > XY_TOLERANCE) { valid = 0; }
#endif

   y = (ADC_MAX_VALUE - samples[NUMSAMPLES/2]);

   // Set X+ to ground
   pinMode(_xp, OUTPUT);


   pinSetLow(_xp);
   pinSetHigh(_ym);
   pinSetLow(_yp);


   pinMode(_yp, INPUT);

   int z1 = analogRead(_xm);
   int z2 = analogRead(_yp);

   if (_rxplate != 0) {
     // now read the x
     float rtouch;
     rtouch = z2;
     rtouch /= z1;
     rtouch -= 1;
     rtouch *= x;
     rtouch *= _rxplate;
     rtouch /= ADC_MAX_VALUE + 1;

     z = rtouch;
   } else {
     z = (ADC_MAX_VALUE - (z2 - z1));
   }

   if (!valid) {
     z = -1;
   }

   _prevAction = TOUCH_PRESSURE;

   return TSPoint(x, y, z);
}

TSPoint TouchScreen::getPoint(bool withPressure) {
  int x, y, z;

  if (_prevAction == TOUCH_X) {
    x = readTouchX();
#if (NUMSAMPLES == 2)
    if (abs(x - readTouchX()) > XY_TOLERANCE)
      return TSPoint(0,0,-1);
#endif
    if (withPressure)
      z = pressure();
    y = readTouchY();
#if (NUMSAMPLES == 2)
    if (abs(y - readTouchY()) > XY_TOLERANCE)
      return TSPoint(0,0,-1);
#endif
  } else {
    y = readTouchY();
#if (NUMSAMPLES == 2)
    if (abs(y - readTouchY()) > XY_TOLERANCE)
      return TSPoint(0,0,-1);
#endif
    if (withPressure)
      z = pressure();
    x = readTouchX();
#if (NUMSAMPLES == 2)
    if (abs(x - readTouchX()) > XY_TOLERANCE)
      return TSPoint(0,0,-1);
#endif
  }

  return TSPoint(x, y, z);
}

int TouchScreen::readTouchX(void) {
if (_prevAction != TOUCH_X) {
  _prevAction = TOUCH_X;

  pinMode(_yp, INPUT);
  pinMode(_ym, INPUT);

  pinMode(_xp, OUTPUT);
  pinMode(_xm, OUTPUT);


  pinSetHigh(_xp);
  pinSetLow(_xm);
  pinSetLow(_yp);
  pinSetLow(_ym);

  }
  return (ADC_MAX_VALUE - analogRead(_yp));
}

int TouchScreen::readTouchY(void) {
  if (_prevAction != TOUCH_Y) {
    _prevAction = TOUCH_Y;

    pinMode(_xp, INPUT);
    pinMode(_xm, INPUT);

    pinMode(_yp, OUTPUT);
    pinMode(_ym, OUTPUT);


    pinSetHigh(_yp);
    pinSetLow(_ym);
    pinSetLow(_xp);
    pinSetLow(_xm);

  }
  return (ADC_MAX_VALUE - analogRead(_xm));
}

uint16_t TouchScreen::pressure(void) {
  if (_prevAction != TOUCH_PRESSURE) {
    _prevAction = TOUCH_PRESSURE;

    pinMode(_xm, INPUT);
    pinMode(_yp, INPUT);

    pinMode(_xp, OUTPUT);
    pinMode(_ym, OUTPUT);


    pinSetLow(_xp);
    pinSetHigh(_ym);
    // ScruffR: Since Spark Core has no PULL UPs with INPUT - not needed
    pinSetLow(_xm);
    pinSetLow(_yp);

  }

  int z1 = analogRead(_xm);
  int z2 = analogRead(_yp);

  if (_rxplate != 0) {
    // now read the x
    float rtouch;
    rtouch = z2;
    rtouch /= z1;
    rtouch -= 1;
    rtouch *= readTouchX();
    rtouch *= _rxplate;
    rtouch /= ADC_MAX_VALUE + 1;

    return rtouch;
  } else {
    return (ADC_MAX_VALUE - (z2 - z1));
  }
}