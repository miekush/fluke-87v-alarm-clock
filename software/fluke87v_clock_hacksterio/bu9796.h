#ifndef _BU9796_H_
#define _BU9796_H_

#include <Arduino.h>
#include <Wire.h>

class BU9796
{
    public:
      BU9796(void);
      void begin(void);
      void clearBuffer(void);
      void clearDigitBuffer(void);
      void update(void);
      void writeDigit(int digit, uint8_t character);
      void writeValue(int value, bool leading = 0);
      void writeChar(int digit1, int digit2, int digit3, int digit4);
      void setDecimalPoint(int digit);
      void setMinusSign(void);
      void setHoldSymbol(void);
      void setBarGraph(int value);
      void setSegment(int byte, int value);
      void fill(bool state = 1);
      void blink(int ms);
    private:
      //nothing to see here...
};

#endif