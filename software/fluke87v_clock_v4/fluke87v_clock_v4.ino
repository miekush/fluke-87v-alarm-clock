#include "bu9796.h"
#include "RTClib.h"
#include "TimerOne.h"
#include "EEPROM.h"

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

//format:
//{pin number, current state, previous state, press time}
uint8_t switch_pins[8][4] =
{
  {SHIFT_SW_PIN,1,1,0},
  {MINMX_SW_PIN,1,1,0},
  {RANGE_SW_PIN,1,1,0},
  {HOLD_SW_PIN,1,1,0},
  {BKLT_SW_PIN,1,1,0},
  {BEEP_SW_PIN,1,1,0},
  {REL_SW_PIN,1,1,0},
  {HZ_SW_PIN,1,1,0}
};

BU9796 display;
RTC_DS3231 rtc;

bool rtc_interruptFlag = false;
bool alarmFlag = false;

bool beepFlag = false;
bool doubleBeepFlag = false;

bool holdFlag = false;

bool alarmSettingsMode = false;
bool setAlarmFlag = false;
bool updateAlarmDisplay = false;
uint8_t alarmTimeInterval = 30;

bool barGraphFlag = true;

//wakeup alarm
DateTime alarm = DateTime(2022, 1, 23, 6, 30, 0);

void buzzerBeep(int ms, int count = 1);

void setup()
{
  //init uart
  Serial.begin(9600);

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

  //attach interrupt to RTC INT pin
  attachInterrupt(digitalPinToInterrupt(RTC_INT_PIN), rtcISR, FALLING);

  //clear alarms
  rtc.clearAlarm(1);
  rtc.clearAlarm(2);

  //turn off squarewave output
  rtc.writeSqwPinMode(DS3231_OFF);

  //rtc calibration mode
  if(!digitalRead(MINMX_SW_PIN))
  {
    //write CAL to display
    display.writeChar(31,12,10,19);
    display.update();

    while(1)
    {
      if(Serial.available())
      {
        String datetime = Serial.readStringUntil('\n');
        char datetime_array[datetime.length()+1];
        datetime.toCharArray(datetime_array, datetime.length());

        //Serial datetime format:
        //YYYYMMDDHHMMSSWWyyy\n
        //YYYY -> year
        //MM   -> month
        //DD   -> day
        //HH   -> hour
        //MM   -> minute
        //SS   -> second
        //WW   -> weekday
        //yy   -> yearday (1-365)

        int year = ((datetime_array[0] - '0') * 1000) + ((datetime_array[1] - '0') * 100) + ((datetime_array[2] - '0') * 10) + (datetime_array[3] - '0');
        int month = ((datetime_array[4] - '0') * 10) + (datetime_array[5] - '0');
        int day = ((datetime_array[6] - '0') * 10) + (datetime_array[7] - '0');
        int hour = ((datetime_array[8] - '0') * 10) + (datetime_array[9] - '0');
        int min = ((datetime_array[10] - '0') * 10) + (datetime_array[11] - '0');
        int second = ((datetime_array[12] - '0') * 10) + (datetime_array[13] - '0');
        //int weekday = ((datetime_array[14] - '0') * 10) + (datetime_array[15] - '0');
        //int yearday = ((datetime_array[16] - '0') * 100) + ((datetime_array[17] - '0') * 10) + (datetime_array[18] - '0');

        rtc.adjust(DateTime(year, month, day, hour, min, second));

        flashBacklight(100);
      }
    }
  }  

  //create timer for polling switches
  Timer1.initialize(100000); //100ms
  Timer1.attachInterrupt(pollSwitches);

  //read alarm time from EEPROM
  uint8_t hours = EEPROM.read(0);
  uint8_t minutes = EEPROM.read(1);

  uint8_t remainder = minutes % alarmTimeInterval;

  if(remainder != 0)
  {
    //round up
    if(remainder >= (alarmTimeInterval/2))
    {
      minutes = (minutes-remainder) + alarmTimeInterval;
    }
    //round down
    else
    {
      minutes = minutes-remainder;
    }
  }

  if(hours != 255 && minutes != 255)
  {
    alarm = DateTime(2022, 1, 23, hours, minutes, 0);
  }

  //disable alarm on startup
  rtc.disableAlarm(2);

  //configure RTC alarm every minute
  rtc.setAlarm1(rtc.now(), DS3231_A1_PerSecond);

  //write latest rtc time to display
  updateDisplayTime(rtc.now());
}

void loop()
{
  if(alarmSettingsMode && updateAlarmDisplay)
  {
    updateDisplayTime(alarm);
    display.blink(2000);
    updateAlarmDisplay = false;
  }
  else if(rtc_interruptFlag && !alarmSettingsMode)
  {
    //get latest time from RTC and write to display
    updateDisplayTime(rtc.now());

    //reset flag
    rtc_interruptFlag = false;

    //alarm 2 caused interrupt
    if(rtc.alarmFired(2))
    {
      //clear alarm
      rtc.clearAlarm(1);
      rtc.clearAlarm(2);

      if(alarmFlag)
      {
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
    }
    //alarm 1 caused interrupt
    else
    {
      //clear alarm
      rtc.clearAlarm(1);
    }
  }

  //single beep request (i.e. button press)
  if(beepFlag)
  {
    buzzerBeep(100);
    beepFlag = false;
  }

  if(doubleBeepFlag)
  {
    buzzerBeep(100, 2);
    doubleBeepFlag = false;
  }

  if(holdFlag)
  {
    displayBatteryVoltage();
    delay(500);
  }
  
  if(setAlarmFlag)
  {
    if(alarmFlag)
    {
      //write new alarm time to RTC
      rtc.setAlarm2(alarm, DS3231_A2_Hour);
      //write new alarm time to EEPROM
      EEPROM.update(0, alarm.hour());
      EEPROM.update(1, alarm.minute());
      //turn off blink
      display.blink(0);
    }
    else
    {
      rtc.disableAlarm(2);
    }
    setAlarmFlag = false;
  }
}

void rtcISR()
{
  rtc_interruptFlag = true;
}

void updateDisplayTime(DateTime time)
{
  //write time to display
  display.writeValue((time.twelveHour() * 100) + time.minute());
  display.setDecimalPoint(3);

  //check for if time is PM
  if(time.isPM())
  {
    //indicate PM time
    display.setMinusSign();
  }

  //check if alarm is set
  if(alarmFlag)
  {
    //indicate HOLD symbol
    display.setHoldSymbol();
  }

  //check if bargraph is enabled
  if(barGraphFlag && !alarmSettingsMode && time.second() != 0)
  {
    display.setBarGraph(time.second()/2);
  }

  //display.setSegment(7, 7);

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
  for(int i=0; i<8; i++)
  {
    //read current values of all switches
    switch_pins[i][1] = digitalRead(switch_pins[i][0]);

    switch(i)
    {
      case SHIFT_SW:
        //falling edge
        if(switch_pins[i][1] == LOW && switch_pins[i][2] == HIGH)
        {
          holdFlag = true;
          
          if(alarmSettingsMode)
          {
            setAlarmFlag = true;
            alarmSettingsMode = false;
            doubleBeepFlag = true;
          }
        }
        //rising edge
        else if(switch_pins[i][1] == HIGH && switch_pins[i][2] == LOW)
        {
          holdFlag = false;
          rtc_interruptFlag = true;
          //reset counter
          switch_pins[i][3] = 0;
        }
        break;
      case MINMX_SW:
        break;
      case RANGE_SW:
        //falling edge
        if(switch_pins[i][1] == LOW && switch_pins[i][2] == HIGH && alarmSettingsMode)
        {
          alarm = alarm + TimeSpan(0,0,alarmTimeInterval,0);
          updateAlarmDisplay = true;
          beepFlag = true;
        }
        break;
      case HOLD_SW:
        break;
      case BKLT_SW:
        //falling edge
        if(switch_pins[i][1] == LOW && switch_pins[i][2] == HIGH)
        {
          toggleBacklight();
          beepFlag = true;
        }
        break;
      case BEEP_SW:
        //falling edge
        if(switch_pins[i][1] == LOW && switch_pins[i][2] == HIGH)
        {
        }
        //still pressed
        else if(switch_pins[i][1] == LOW && switch_pins[i][2] == LOW)
        {
          //increment counter
          switch_pins[i][3]++;

          if(switch_pins[i][3] > 15 && alarmSettingsMode == false && alarmFlag == true)
          {
            doubleBeepFlag = true;
            alarmSettingsMode = true;
            updateAlarmDisplay = true;
          }
        }
        //rising edge
        else if(switch_pins[i][1] == HIGH && switch_pins[i][2] == LOW)
        {
          if(switch_pins[i][3] <= 15)
          {
            if(alarmSettingsMode)
            {
              setAlarmFlag = true;
              alarmSettingsMode = false;
              doubleBeepFlag = true;
            }
            else
            {
              alarmFlag = !alarmFlag;
              setAlarmFlag = true;
              beepFlag = true;
            }
            rtc_interruptFlag = true;
          }
          //reset counter
          switch_pins[i][3] = 0;
        }
        break;
      case REL_SW:
        //falling edge
        if(switch_pins[i][1] == LOW && switch_pins[i][2] == HIGH && alarmSettingsMode)
        {
          alarm = alarm - TimeSpan(0,0,alarmTimeInterval,0);
          updateAlarmDisplay = true;
          beepFlag = true;
        }
        break;
      case HZ_SW:
        break;
      default:
        break;
    }
  }

  for(int j=0; j<8; j++)
  {
    //copy value of current to previous
    switch_pins[j][2] = switch_pins[j][1];
  }
}

void toggleBacklight(void)
{
  //toggle bklt pin
  digitalWrite(BKLT_PIN, !digitalRead(BKLT_PIN));
}

void flashBacklight(int ms)
{
  digitalWrite(BKLT_PIN, HIGH);
  delay(ms);
  digitalWrite(BKLT_PIN, LOW);
}

void buzzerBeep(int ms, int count)
{
  for(int i=0; i<count; i++)
  {
    digitalWrite(BZR_PIN, HIGH);
    delay(ms);
    digitalWrite(BZR_PIN, LOW);
    delay(ms);
  }
}
