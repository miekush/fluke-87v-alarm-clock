#include "bu9796.h"

//masks for digit segments in bu9796 ram (adjust depending on your display)
const uint8_t segment_map[5][7][2] =
{
  {
    {0, 0},     //1a
    {4, 0b00100000},  //1b
    {4, 0b00100000},  //1c
    {0, 0},     //1d
    {0, 0},     //1e
    {0, 0},     //1f
    {0, 0}      //1g
  },
  {
    {3, 0b10000000},  //2a
    {3, 0b01000000},  //2b
    {3, 0b00100000},  //2c
    {3, 0b00010000},  //2d
    {3, 0b00000010},  //2e
    {3, 0b00001000},  //2f
    {3, 0b00000100}   //2g
  },
  {
    {2, 0b10000000},  //3a
    {2, 0b01000000},  //3b
    {2, 0b00100000},  //3c
    {2, 0b00010000},  //3d
    {2, 0b00000010},  //3e
    {2, 0b00001000},  //3f
    {2, 0b00000100}   //3g
  },
  {
    {1, 0b00001000},  //4a
    {1, 0b00000100},  //4b
    {1, 0b00000010},  //4c
    {1, 0b00000001},  //4d
    {1, 0b00100000},  //4e
    {1, 0b10000000},  //4f
    {1, 0b01000000}   //4g
  },
  {
    {0, 0b00001000},  //5a
    {0, 0b00000100},  //5b
    {0, 0b00000010},  //5c
    {0, 0b00000001},  //5d
    {0, 0b00100000},  //5e
    {0, 0b10000000},  //5f
    {0, 0b01000000}   //5g
  }
};

//map for decimal points
const uint8_t dp_map[4][2] =
{
  {3, 0b00000001},    //Digit 1
  {2, 0b00000001},    //Digit 2
  {1, 0b00010000},    //Digit 3
  {0, 0b00010000}     //Digit 4
};

//map for bar graph
const uint8_t bar_graph_map[30][2] =
{
  {5, 0b00000001},    //b5
  {6, 0b00010000},    //b11
  {6, 0b00100000},    //b10
  {6, 0b01000000},    //b9
  {6, 0b10000000},    //b8
  {6, 0b00000100},    //b14
  {6, 0b00000010},    //b13
  {6, 0b00000001},    //b12
  {8, 0b00000001},    //auto
  {8, 0b00000010},    //x28
  {8, 0b00000100},    //b16
  {8, 0b00001000},    //bt2
  {8, 0b01000000},    //b18
  {8, 0b00100000},    //b19
  {8, 0b00010000},    //b20
  {7, 0b00000001},    //b21
  {7, 0b00000010},    //b22
  {7, 0b00000100},    //b23
  {7, 0b00001000},    //bt3
  {7, 0b01000000},    //b25
  {7, 0b00100000},    //b26
  {7, 0b00010000},    //b27
  {9, 0b00010000},    //b28
  {9, 0b00100000},    //b29
  {9, 0b01000000},    //b30
  {9, 0b10000000},    //b31
  {9, 0b00001000},    //bt4
  {9, 0b00000100},    //B32
  {9, 0b00000010},    //x26
  {9, 0b10000001}     //man
};

//masks for digit pins
const uint8_t digit_mask[5] =
{
  0b11101111,   //digit 5
  0b11101111,   //digit 4
  0b11111110,   //digit 3
  0b11111110,   //digit 2
  0b00100000    //digit 1
};

//map for segment pins -> digit values
const uint8_t character_map[16][7] =
{
  //{a,b,c,d,e,f,g}
  {1,1,1,1,1,1,0},  //0
  {0,1,1,0,0,0,0},  //1
  {1,1,0,1,1,0,1},  //2
  {1,1,1,1,0,0,1},  //3
  {0,1,1,0,0,1,1},  //4
  {1,0,1,1,0,1,1},  //5
  {1,0,1,1,1,1,1},  //6
  {1,1,1,0,0,0,0},  //7
  {1,1,1,1,1,1,1},  //8
  {1,1,1,0,0,1,1},  //9
  {1,1,1,0,1,1,1},  //A
  {0,0,1,1,1,1,1},  //b
  {1,0,0,1,1,1,0},  //C
  {0,1,1,1,1,0,1},  //d
  {1,0,0,1,1,1,1},  //E
  {1,0,0,0,1,1,1}   //F
};

//buffer to store display ram
uint8_t buffer[10] = {0,0,0,0,0,0,0,0,0,0};

//constructor
BU9796::BU9796(void)
{
}

//init function for bu9796
void BU9796::begin(void)
{
  //wait a bit for BU9796
  delay(1);

  //init i2c bus
  Wire.begin();

  //create STOP condition
  Wire.endTransmission();
  //create START condition
  Wire.beginTransmission(0x3e);
  //ICSET -> SW RESET
  Wire.write(0b11101010);
  //BLKCTL
  Wire.write(0b11110000);
  //DISCTL
  Wire.write(0b10111110);
  //ICSET -> Internal Oscillator
  Wire.write(0b11101000);
  //ADSET -> Address 0
  Wire.write(0b00000000);

  //write display data to RAM (00h -> 13h)
  for(int i=0; i<10; i++)
  {
    Wire.write(0b00000000);
  }
  
  //create STOP condition
  Wire.endTransmission();
  //create START condition
  Wire.beginTransmission(0x3e);
  //MODESET -> Display ON
  Wire.write(0b11001000);
  //create STOP condition
  Wire.endTransmission();
}

//clear entire buffer
void BU9796::clearBuffer(void)
{
  //clear buffer
  for(int i=0; i<10; i++)
  {
    buffer[i] = 0;
  }  
}

//clear buffer area for segments only
void BU9796::clearDigitBuffer(void)
{
  //clear buffer for digits
  for(int i=0; i<5; i++)
  {
    buffer[i] &= ~(digit_mask[i]);
  }
}

//write buffer data to bu9796 ram
void BU9796::update(void)
{
  //create START condition
  Wire.beginTransmission(0x3e);
  //ICSET -> SW RESET
  Wire.write(0b11101010);
  //BLKCTL
  Wire.write(0b11110000);
  //DISCTL
  Wire.write(0b10111110);
  //ICSET -> Internal Oscillator
  Wire.write(0b11101000);
  //ADSET -> Address 0
  Wire.write(0b00000000);

  //Display Data (00h -> 13h)
  for(int i=0; i<10; i++)
  {
    Wire.write(buffer[i]);
  }
  
  //create STOP condition
  Wire.endTransmission();
  //create START condition
  Wire.beginTransmission(0x3e);
  //MODESET -> Display ON
  Wire.write(0b11001000);
  //create STOP condition
  Wire.endTransmission();
}

//write single digit to display segments
void BU9796::writeDigit(int digit, uint8_t character)
{
  for(int i=0; i<7; i++)
  {
    if(character_map[character][i])
    {
      //set segment bit in display RAM buffer
      buffer[segment_map[digit][i][0]] |= segment_map[digit][i][1];
    }
  }  
}

//write 1-5 digit value to display segments
void BU9796::writeValue(int value, bool leading)
{
  //clear display buffer
  clearBuffer();

  if(!leading && !((value / 1000) % 10))
  {
    //write three digits
    if((value / 100) % 10)
    {
      writeDigit(2, (value / 100) % 10);
      writeDigit(3, (value / 10) % 10);
      writeDigit(4, value % 10);      
    }
    //write two digits
    else if((value / 10) % 10)
    {
      writeDigit(3, (value / 10) % 10);
      writeDigit(4, value % 10);
    }
    //write one difit
    else
    {
      writeDigit(4, value % 10);
    }
  }
  else
  {
    writeDigit(1, (value / 1000) % 10);
    writeDigit(2, (value / 100) % 10);
    writeDigit(3, (value / 10) % 10);
    writeDigit(4, value % 10);
  }
}

//enable decimal point
void BU9796::setDecimalPoint(int digit)
{
  //set segment bit in display RAM buffer
  buffer[dp_map[digit-1][0]] |= dp_map[digit-1][1];
}

//enable minus sign
void BU9796::setMinusSign(void)
{
  //set segment (X36) bit in display RAM buffer
  buffer[4] |= 0b01000000;
}

//enable hold symbol
void BU9796::setHoldSymbol(void)
{
  //set segment (X2) bit in display RAM buffer
  buffer[4] |= 0b10000000;
}

//set digital bar graph
void BU9796::setBarGraph(int value)
{
  if(value<30)
  {
    for(int i=0; i<(value+1); i++)
    {
      //set segment bit in display buffer
      buffer[bar_graph_map[i][0]] |= bar_graph_map[i][1];
    }
  }
}

//set individual display segment
void BU9796::setSegment(int byte, int value)
{
  buffer[byte] |= (1 << value);
}

//control all pixels/segments (APCTL)
void BU9796::fill(bool state)
{
  //create START condition
  Wire.beginTransmission(0x3e);  

  //all pixel ON
  if(state)
  {
    Wire.write(0b11111110);
  }
  //all pixel OFF
  else
  {
    Wire.write(0b11111101);
  }

  //create STOP condition
  Wire.endTransmission();  
}

//set bu9796 blink mode (BLKCTL)
void BU9796::blink(int ms)
{
  //create START condition
  Wire.beginTransmission(0x3e);
  
  //configure BLKCTL register
  switch(ms)
  {
    //OFF
    case 0:
      Wire.write(0b11110000);
      break;
    //0.5 Hz
    case 500:
      Wire.write(0b11110001);
      break;
    //1 Hz
    case 1000:
      Wire.write(0b11110010);
      break;
    //2 Hz
    case 2000:
      Wire.write(0b11110011);
      break;
    //OFF
    default:
      Wire.write(0b11110000);
      break;
  }
  
  //create STOP condition
  Wire.endTransmission();
}
