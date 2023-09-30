#include "bu9796.h"

#define BKLT_PIN        3

BU9796 display;

void setup()
{
  pinMode(BKLT_PIN, OUTPUT);
  digitalWrite(BKLT_PIN, HIGH);

  display.begin();
}

void loop()
{
  for(int i=0; i<30; i++)
  {
    display.setBarGraph(i);
    display.update();
    delay(100);
  }
  
  display.clearBuffer();
  display.update();
}
