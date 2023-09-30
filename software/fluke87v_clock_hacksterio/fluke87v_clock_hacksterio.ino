#include "bu9796.h"

//output pins
#define BKLT_PIN        3

BU9796 display;

void setup()
{
  //init peripheral devices
  display.begin();

  //init output pins
  pinMode(BKLT_PIN, OUTPUT);

  //turn backlight off by default
  digitalWrite(BKLT_PIN, HIGH);
}

void loop()
{
  //hACK
  display.clearBuffer();
  display.writeChar(16, 10, 12, 20);
  display.update();
  delay(1000);
  //STER
  display.clearBuffer();
  display.writeChar(27, 28, 14, 26);
  display.update();
  delay(1000);
  //.io
  display.clearBuffer();
  display.setDecimalPoint(1);
  display.writeChar(18, 23, 33, 33);
  display.update();
  delay(2500);
}
