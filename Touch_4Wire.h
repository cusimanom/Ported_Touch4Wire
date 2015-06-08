// Touch screen library with X Y and Z (pressure) readings as well
// as oversampling to avoid 'bouncing'
// (c) ladyada / adafruit
// Code under MIT License
//
// ported for Spark Core by ScruffR Nov. 2014 additions Wesner0019 June 2015
// https://github.com/ScruffR/touch_4wire/tree/master/firmware

#ifndef _ADAFRUIT_TOUCH4WIRE_H_
#define _ADAFRUIT_TOUCH4WIRE_H_

#include "application.h"
#define ADC_MAX_VALUE (0x0FFF)
#define XY_TOLERANCE 20

#if  (PLATFORM_ID) == 6
    #define pinSetLow(pin) PIN_MAP[pin].gpio_peripheral->BSRRH = PIN_MAP[pin].gpio_pin
    #define pinSetHigh(pin) PIN_MAP[pin].gpio_peripheral->BSRRL = PIN_MAP[pin].gpio_pin
#else
    #define pinSetHigh(pin) PIN_MAP[pin].gpio_peripheral->BSRR = PIN_MAP[pin].gpio_pin
    #define pinSetLow(pin)  PIN_MAP[pin].gpio_peripheral->BRR = PIN_MAP[pin].gpio_pin
#endif
 

typedef enum TOUCH_ACTIONS {
  TOUCH_UNDEF,
  TOUCH_X,
  TOUCH_Y,
  TOUCH_PRESSURE
};

class TSPoint {
 public:
  TSPoint(void);
  TSPoint(int16_t x, int16_t y, int16_t z);

  bool operator==(TSPoint);
  bool operator!=(TSPoint);

  int16_t x, y, z;
};

class TouchScreen {
 public:
  TouchScreen(uint8_t xp, uint8_t yp, uint8_t xm, uint8_t ym);
  TouchScreen(uint8_t xp, uint8_t yp, uint8_t xm, uint8_t ym, uint16_t rx);

  bool isTouching(void);
  uint16_t pressure(void);
  int readTouchY();
  int readTouchX();
  TSPoint getPoint();
  TSPoint getPoint(bool withPressure);
  int16_t pressureThreshhold;

private:
  uint8_t _yp, _ym, _xm, _xp;
  uint16_t _rxplate;
  TOUCH_ACTIONS _prevAction = TOUCH_UNDEF;
};

#endif