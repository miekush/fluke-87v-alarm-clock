#include "bu9796.h"
#include "RTClib.h"
#include "TimerOne.h"

//output pins
#define STAT_LED_PIN    4
#define BKLT_PIN        3
#define BZR_PIN         16

//digital input pins 
#define SHIFT_SW_PIN    8
#define MINMX_SW_PIN    7
#define RANGE_SW_PIN    6
#define HOLD_SW_PIN     5
#define BKLT_SW_PIN     15
#define BEEP_SW_PIN     14
#define REL_SW_PIN      10
#define HZ_SW_PIN       9
#define RTC_INT_PIN     2

//analog input pins
#define BAT_V_PIN       A3

//button switch pins
#define SHIFT_SW        0
#define MINMX_SW        1
#define RANGE_SW        2
#define HOLD_SW         3
#define BKLT_SW         4
#define BEEP_SW         5
#define REL_SW          6
#define HZ_SW           7

BU9796 display;
RTC_DS3231 rtc;

bool rtc_interruptFlag = false;
bool prev_bkltSWState = true;
bool current_bkltSWState = true;
bool prev_hold_SWState = true;
bool current_hold_SWState = true;
bool beepFlag = false;
bool holdFlag = false;

void setup()
{
  //init peripheral devices
  display.begin();
  rtc.begin();

  //disable 32 kHz output
  rtc.disable32K();

  //init output pins
  pinMode(STAT_LED_PIN, OUTPUT);
  pinMode(BKLT_PIN, OUTPUT);
  pinMode(BZR_PIN, OUTPUT);

  //init input pins
  pinMode(SHIFT_SW_PIN, INPUT);
  pinMode(MINMX_SW_PIN, INPUT);
  pinMode(RANGE_SW_PIN, INPUT);
  pinMode(HOLD_SW_PIN, INPUT);
  pinMode(BKLT_SW_PIN, INPUT);
  pinMode(BEEP_SW_PIN, INPUT);
  pinMode(REL_SW_PIN, INPUT);
  pinMode(HZ_SW_PIN, INPUT);
  pinMode(RTC_INT_PIN, INPUT);

  //turn backlight off by default
  digitalWrite(BKLT_PIN, LOW);

  //turn buzzer off
  digitalWrite(BZR_PIN, LOW);

  //get current state of the backlight switch
  prev_bkltSWState = digitalRead(BKLT_SW_PIN);

  //attach interrupt to RTC INT pin
  attachInterrupt(digitalPinToInterrupt(RTC_INT_PIN), rtcISR, FALLING);

  //create timer for polling switches
  Timer1.initialize(100000); //100ms
  Timer1.attachInterrupt(pollSwitches);

  //clear alarms
  rtc.clearAlarm(1);
  rtc.clearAlarm(2);

  //turn off squarewave output
  rtc.writeSqwPinMode(DS3231_OFF);

  rtc.setAlarm1(DateTime(2022, 1, 23, 6, 30, 0), DS3231_A1_Hour);

  //configure RTC alarm every minute
  rtc.setAlarm2(rtc.now(), DS3231_A2_PerMinute);

  //write latest rtc time to display
  updateDisplayTime();
}

void loop()
{
  //check for RTC interrupt
  if(rtc_interruptFlag)
  {
    //alarn 1 caused interrupt
    if(rtc.alarmFired(1))
    {
      //clear alarm
      rtc.clearAlarm(2);

      //reset flag
      rtc_interruptFlag = false;

      //get latest time from RTC and write to display
      updateDisplayTime();

      //blink display segments (1 Hz)
      display.blink(1000);

      //alarm loop until reset
      while(1)
      {
        toggleBacklight();
        buzzerBeep(1000);
        toggleBacklight();
        delay(1000);
      }
    }
    //alarm 2 caused interrupt
    else
    {
      //clear alarm
      rtc.clearAlarm(2);

      //get latest time from RTC and write to display
      updateDisplayTime();

      //reset flag
      rtc_interruptFlag = false;      
    }
  }

  //check for single beep request (i.e. button press)
  if(beepFlag)
  {
    buzzerBeep(100);
    beepFlag = false;
  }

  if(holdFlag)
  {
    displayBatteryVoltage();
    delay(500);
  }
}

void rtcISR()
{
  rtc_interruptFlag = true;
}

void updateDisplayTime(void)
{
  //get latest time from RTC
  DateTime now = rtc.now();

  //write time to display
  display.writeValue((now.twelveHour() * 100) + now.minute());
  display.setDecimalPoint(3);
  display.update();
}

void displayBatteryVoltage(void)
{
  //read battery voltage
  int battV = 10.1 * (3300.0 * (analogRead(BAT_V_PIN) / 1023.0));
  display.writeValue(battV);
  display.update();
}

void pollSwitches(void)
{
  //read current switch status
  current_bkltSWState = digitalRead(BKLT_SW_PIN);
  current_hold_SWState = digitalRead(SHIFT_SW_PIN);

  //falling edge
  if(current_bkltSWState == LOW && prev_bkltSWState == HIGH)
  {
    toggleBacklight();
    beepFlag = true;
  }

  //falling edge
  if(current_hold_SWState == LOW && prev_hold_SWState == HIGH)
  {
    holdFlag = true;
  }
  //rising edge
  else if(current_hold_SWState == HIGH && prev_hold_SWState == LOW)
  {
    holdFlag = false;
    rtc_interruptFlag = true;
  }

  prev_bkltSWState = current_bkltSWState;
  prev_hold_SWState = current_hold_SWState;
}

void toggleBacklight(void)
{
  //toggle bklt pin
  digitalWrite(BKLT_PIN, !digitalRead(BKLT_PIN));
}

void buzzerBeep(int ms)
{
  digitalWrite(BZR_PIN, HIGH);
  delay(ms);
  digitalWrite(BZR_PIN, LOW);
}
