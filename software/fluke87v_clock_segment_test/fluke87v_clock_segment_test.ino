#include "bu9796.h"

#define BKLT_PIN        3

BU9796 display;

void setup()
{
  Serial.begin(115200);
  Serial.println("Starting segment test...");

  pinMode(BKLT_PIN, OUTPUT);
  digitalWrite(BKLT_PIN, HIGH);

  display.begin();
}

void loop()
{
  display.clearBuffer();
  
  for(int i=0; i<10; i++)
  {
    for(int j=0; j<8; j++)
    {
      Serial.print("Byte:");
      Serial.print(i);
      Serial.print(" Bit:");
      Serial.println(j);

      display.setSegment(i, 1<<j);
      display.update();
      delay(1000);
    }
  }
}
