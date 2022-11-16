
//Libraries Needed
//. Adafruit BusIO
//. Adafruit Unified Sensor
//. DHT sensor library
//  RTClib
//  TFT Touch Shield v2.0

#include "DHT.h"
#include <stdint.h>
#include <TFTv2.h>
#include <stdint.h>
#include <SeeedTouchScreen.h>
#include "RTClib.h"
#include "Wire.h"

#define AT24C32_Address 0x57

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

#if defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) // mega
#define YP A2   // must be an analog pin, use "An" notation!
#define XM A1   // must be an analog pin, use "An" notation!
#define YM 54   // can be a digital pin, this is A0
#define XP 57   // can be a digital pin, this is A3 

#elif defined(__AVR_ATmega32U4__) // leonardo
#define YP A2   // must be an analog pin, use "An" notation!
#define XM A1   // must be an analog pin, use "An" notation!
#define YM 18   // can be a digital pin, this is A0
#define XP 21   // can be a digital pin, this is A3 
#elif defined(ARDUINO_SAMD_VARIANT_COMPLIANCE) // samd21

#define YP A2   // must be an analog pin, use "An" notation!
#define XM A1   // must be an analog pin, use "An" notation!
#define YM A4   // can be a digital pin, this is A0 
#define XP A3   // can be a digital pin, this is A3 

#else //168, 328, something else
#define YP A2   // must be an analog pin, use "An" notation!
#define XM A1   // must be an analog pin, use "An" notation!
#define YM 14   // can be a digital pin, this is A0
#define XP 17   // can be a digital pin, this is A3 

#endif

#define TS_MINX 116*2
#define TS_MAXX 890*2
#define TS_MINY 83*2
#define TS_MAXY 913*2

#define DHT1_PIN 22
#define DHT2_PIN 23

#define LIGHTS_PIN 10

#define ROCK_TEMP_PIN 9

#define UNUSED_CNTRL_PIN 30
#define FAN_CNTRL_PIN 32
#define HUMIDITY_CNTRL_PIN 34
#define TEMP_CNTRL_PIN 36

#define DHTTYPE DHT22

#define TEMP_DRIFT (1.0)

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// The 2.8" TFT Touch shield has 300 ohms across the X plate
TouchScreen ts = TouchScreen(XP, YP, XM, YM);

DHT dht1(DHT1_PIN, DHTTYPE);
DHT dht2(DHT2_PIN, DHTTYPE);

RTC_DS3231 rtc;

uint8_t Desired_Temp = 80;
uint8_t Desired_Humidity = 80;

float Current_Temp_1 = 88.8;
float Current_Temp_2 = 88.8;

float Current_Humidity_1 = 88.8;
float Current_Humidity_2 = 88.8;

uint8_t pwmCount = 0;
uint8_t pwmDIR = 0;
uint8_t pwmTemp = 0;
uint8_t loopCount = 0;
uint8_t state = 0;

DateTime prev_time;

#define OVERSAMPLENR 16
#define OV(N) int16_t((N)*(OVERSAMPLENR))

// R25 = 100 kOhm, beta25 = 3974 K, 4.7 kOhm pull-up, Honeywell 135-104LAG-J01
const short temptable[][2] PROGMEM = {
  { OV(   1), 941 },
  { OV(  19), 362 },
  { OV(  37), 299 }, // top rating 300C
  { OV(  55), 266 },
  { OV(  73), 245 },
  { OV(  91), 229 },
  { OV( 109), 216 },
  { OV( 127), 206 },
  { OV( 145), 197 },
  { OV( 163), 190 },
  { OV( 181), 183 },
  { OV( 199), 177 },
  { OV( 217), 171 },
  { OV( 235), 166 },
  { OV( 253), 162 },
  { OV( 271), 157 },
  { OV( 289), 153 },
  { OV( 307), 149 },
  { OV( 325), 146 },
  { OV( 343), 142 },
  { OV( 361), 139 },
  { OV( 379), 135 },
  { OV( 397), 132 },
  { OV( 415), 129 },
  { OV( 433), 126 },
  { OV( 451), 123 },
  { OV( 469), 121 },
  { OV( 487), 118 },
  { OV( 505), 115 },
  { OV( 523), 112 },
  { OV( 541), 110 },
  { OV( 559), 107 },
  { OV( 577), 105 },
  { OV( 595), 102 },
  { OV( 613),  99 },
  { OV( 631),  97 },
  { OV( 649),  94 },
  { OV( 667),  92 },
  { OV( 685),  89 },
  { OV( 703),  86 },
  { OV( 721),  84 },
  { OV( 739),  81 },
  { OV( 757),  78 },
  { OV( 775),  75 },
  { OV( 793),  72 },
  { OV( 811),  69 },
  { OV( 829),  66 },
  { OV( 847),  62 },
  { OV( 865),  59 },
  { OV( 883),  55 },
  { OV( 901),  51 },
  { OV( 919),  46 },
  { OV( 937),  41 },
  { OV( 955),  35 },
  { OV( 973),  27 },
  { OV( 991),  17 },
  { OV(1009),   1 },
  { OV(1023),   0 } // to allow internal 0 degrees C
};

void draw_main() 
{
  state = 0;
  Tft.fillScreen();
  Tft.drawVerticalLine(0, 0, 320, YELLOW);
  Tft.drawVerticalLine(160, 100, 60, YELLOW);
  Tft.drawVerticalLine(80, 115, 45, YELLOW);
  Tft.drawVerticalLine(160, 180, 60, YELLOW);
  Tft.drawVerticalLine(80, 195, 45, YELLOW);
  Tft.drawVerticalLine(239, 0, 320, YELLOW);
  
  Tft.drawHorizontalLine(0, 0, 240, YELLOW);
  Tft.drawHorizontalLine(0, 79, 240, YELLOW);
  Tft.drawHorizontalLine(0, 100, 240, YELLOW);
  Tft.drawHorizontalLine(0, 115, 240, YELLOW);
  Tft.drawHorizontalLine(0, 159, 240, YELLOW);
  Tft.drawHorizontalLine(0, 180, 240, YELLOW);
  Tft.drawHorizontalLine(0, 195, 240, YELLOW);
  Tft.drawHorizontalLine(0, 239, 240, YELLOW);
  Tft.drawHorizontalLine(0, 319, 240, YELLOW);

  Tft.drawString("11/12/20", 20, 10, 4, WHITE);
  Tft.drawString("12:35:78", 20, 45, 4, WHITE);

  //Display Current Date and Time.
  Tft.fillRectangle(20, 10, 50, 30, BLACK);
  Tft.drawNumber(prev_time.month(), 20, 10, 4, WHITE);
  Tft.fillRectangle(92, 10, 50, 30, BLACK);
  Tft.drawNumber(prev_time.day(), 92, 10, 4, WHITE);
  Tft.fillRectangle(164, 10, 50, 30, BLACK);
  Tft.drawNumber(prev_time.year() - 2000, 164, 10, 4, WHITE);
  
  Tft.fillRectangle(20, 45, 50, 30, BLACK);
  Tft.drawNumber(prev_time.hour(), 20, 45, 4, WHITE);
  Tft.fillRectangle(92, 45, 50, 30, BLACK);
  Tft.drawNumber(prev_time.minute(), 92, 45, 4, WHITE);
  Tft.fillRectangle(164, 45, 50, 30, BLACK);
  Tft.drawNumber(prev_time.second(), 164, 45, 4, WHITE);

  Tft.drawString("Temperature", 55, 82, 2, GREEN);
  Tft.drawString("Current", 5, 105, 1, CYAN);
  Tft.drawFloat(Current_Temp_1, 1, 2, 124, 3, CYAN);
  Tft.drawFloat(Current_Temp_2, 1, 83, 124, 3, CYAN);
  
  Tft.drawString("Desired", 162, 105, 1, WHITE);
  Tft.drawNumber(Desired_Temp, 175, 124, 4, WHITE);

  Tft.drawString("Humidity", 75, 162, 2, GREEN);
  Tft.drawString("Current", 5, 185, 1, CYAN);
  Tft.drawFloat(Current_Humidity_1, 1, 2, 206, 3, CYAN);
  Tft.drawFloat(Current_Humidity_2, 1, 83, 206, 3, CYAN);
  
  Tft.drawString("Desired", 162, 185, 1, WHITE);
  Tft.drawNumber(Desired_Humidity, 175, 206, 4, WHITE);

  Tft.drawString("Rock Temp", 10, 242, 2, GREEN);
  Tft.drawFloat(88.8, 1, 70, 267, 4, CYAN);
}

void draw_set_time() 
{
  state = 1;
  Tft.fillScreen();

  Tft.fillRectangle(20, 5, 50, 30, GREEN);
  Tft.fillRectangle(94, 5, 50, 30, GREEN);
  Tft.fillRectangle(164, 5, 50, 30, GREEN);
  
  Tft.drawString("U",  36, 10, 2, BLACK);
  Tft.drawString("U", 110, 10, 2, BLACK);
  Tft.drawString("U", 180, 10, 2, BLACK);

  Tft.drawNumber(prev_time.month(), 20, 43, 4, WHITE);
  Tft.drawString("/", 70, 43, 4, WHITE);
  Tft.drawNumber(prev_time.day(), 94, 43, 4, WHITE);
  Tft.drawString("/", 140, 43, 4, WHITE);
  Tft.drawNumber(prev_time.year() - 2000, 164, 43, 4, WHITE);

  Tft.fillRectangle(20,  80, 50, 30, GREEN);
  Tft.fillRectangle(94,  80, 50, 30, GREEN);
  Tft.fillRectangle(164, 80, 50, 30, GREEN);

  Tft.drawString("D",  36, 85, 2, BLACK);
  Tft.drawString("D", 110, 85, 2, BLACK);
  Tft.drawString("D", 180, 85, 2, BLACK);

  Tft.fillRectangle(20,  128, 50, 30, GREEN);
  Tft.fillRectangle(94,  128, 50, 30, GREEN);
  Tft.fillRectangle(164, 128, 50, 30, GREEN);

  Tft.drawString("U",  36, 133, 2, BLACK);
  Tft.drawString("U", 110, 133, 2, BLACK);
  Tft.drawString("U", 180, 133, 2, BLACK);

  Tft.drawNumber(prev_time.hour(), 20, 165, 4, WHITE);
  Tft.drawString(":", 70, 165, 4, WHITE);
  Tft.drawNumber(prev_time.minute(), 94, 165, 4, WHITE);
  Tft.drawString(":", 140, 165, 4, WHITE);
  Tft.drawNumber(prev_time.second(), 164, 165, 4, WHITE);

  Tft.fillRectangle(20,  200, 50, 30, GREEN);
  Tft.fillRectangle(94,  200, 50, 30, GREEN);
  Tft.fillRectangle(164, 200, 50, 30, GREEN);

  Tft.drawString("D",  36, 205, 2, BLACK);
  Tft.drawString("D", 110, 205, 2, BLACK);
  Tft.drawString("D", 180, 205, 2, BLACK);

  Tft.fillRectangle(20, 260, 200, 40, GREEN);
  Tft.drawString("DONE", 70, 265, 4, BLACK);
}

void draw_set_temp() 
{
  state = 2;
  Tft.fillScreen();

  Tft.drawString("SET", 85, 25, 4, WHITE);
  Tft.drawString("TEMP", 75, 55, 4, WHITE);

  Tft.drawNumber(Desired_Temp, 25, 145, 6, WHITE);

  Tft.fillRectangle(150, 105, 65, 40, GREEN);
  Tft.drawString("UP", 163, 114, 3, BLACK);

  Tft.fillRectangle(150, 185, 65, 40, GREEN);
  Tft.drawString("DWN", 154, 193, 3, BLACK);

  Tft.fillRectangle(20, 260, 200, 40, GREEN);
  Tft.drawString("DONE", 70, 265, 4, BLACK);
}

void draw_set_hum() 
{
  state = 3;
  Tft.fillScreen();

  Tft.drawString("SET", 85, 25, 4, WHITE);
  Tft.drawString("HUMIDITY", 20, 55, 4, WHITE);

  Tft.drawNumber(Desired_Humidity, 25, 145, 6, WHITE);

  Tft.fillRectangle(150, 105, 65, 40, GREEN);
  Tft.drawString("UP", 163, 114, 3, BLACK);

  Tft.fillRectangle(150, 185, 65, 40, GREEN);
  Tft.drawString("DWN", 154, 193, 3, BLACK);

  Tft.fillRectangle(20, 260, 200, 40, GREEN);
  Tft.drawString("DONE", 70, 265, 4, BLACK);
}

/***************************************************/
//Control Heater here.
/***************************************************/
void control_loop(float rock_temp)
{
  if(rock_temp > 30.0 && rock_temp < 100.0)
  {
    float Current_Temp = -1.0;

    //Use Higher of the 2 Values
    if(Current_Temp_1 > Current_Temp_2)
    {
      Current_Temp = Current_Temp_1;
    }
    else
    {
      Current_Temp = Current_Temp_2;
    }
    
    //Use Higher of the 2 Values
    if(Current_Temp > 20.0 && Current_Temp < 82.0)
    {
      if(Current_Temp > ((float)Desired_Temp + TEMP_DRIFT))
      { //Temp is over set temp plus drift. Turn heater off.
        pwmTemp = 0;
      }
      else if(Current_Temp < ((float)Desired_Temp - (TEMP_DRIFT*2)))
      { //Temp is 2 deg below desired. Turn on heater 400 counts out of 510 (78%)
        // Math = (255 - pwmTemp) * 2
        pwmTemp = 55;
      }
      else if(Current_Temp < ((float)Desired_Temp - TEMP_DRIFT))
      { //Temp is 1 deg below desired. Turn on heater 254 counts out of 510 (50%)
        // Math = (255 - pwmTemp) * 2
        pwmTemp = 128;
      }
      else if(Current_Temp < ((float)Desired_Temp) && Current_Temp > ((float)Desired_Temp - TEMP_DRIFT))
      { //Temp is between set point and drift. Turn on heater 128 counts out of 510 (25%)
        // Math = (255 - pwmTemp) * 2
        pwmTemp = 191;
      }
    }
    else
    {
      pwmTemp = 0;
    }
  }
  else
  {
    pwmTemp = 0;
  }
  
  if(pwmTemp == 0 || pwmTemp == 255)
  {
    digitalWrite(TEMP_CNTRL_PIN, LOW);
  }
  else if(pwmCount >= pwmTemp)
  {
    digitalWrite(TEMP_CNTRL_PIN, HIGH);
  }
  else
  {
    digitalWrite(TEMP_CNTRL_PIN, LOW);    
  }

  if(pwmDIR == 0)
  {
    pwmCount++;

    if(pwmCount == 0xFF)
    {
      pwmDIR = 1;
    }
  }
  else
  {
    pwmCount--;

    if(pwmCount == 0x00)
    {
      pwmDIR = 0;
    }
  }
}

float get_rock_temp()
{
  static int overCount = 0;
  static uint16_t rawValue = 0;
  static float prevTemp = 0.0;

  overCount++;
  rawValue += analogRead(A9);

  if(overCount >= OVERSAMPLENR)
  {
    overCount = 0;
    
    uint8_t len = sizeof(temptable)/sizeof(*temptable);
    uint8_t l = 0, r = len, m;                                           
    for (;;) 
    {
      m = (l + r) >> 1;
      if (m == l || m == r)
      {
        float result = (float)pgm_read_word(&temptable[len-1][1]);
        prevTemp = result * 9.0 / 5.0 + 32.0;
        break;
      }
      uint16_t v00 = pgm_read_word(&temptable[m-1][0]);
      uint16_t v10 = pgm_read_word(&temptable[m-0][0]);
      if (rawValue < v00)
      {
        r = m;
      }
      else if (rawValue > v10) 
      {
        l = m;                                         
      }
      else
      {
        const uint16_t v01 = (uint16_t)pgm_read_word(&temptable[m-1][1]);
        const uint16_t v11 = (uint16_t)pgm_read_word(&temptable[m-0][1]);   
        float result = float(v01) + (float(rawValue) - float(v00)) * (float(v11) - float(v01)) / (float(v10) - float(v00));
        prevTemp = result * 9.0 / 5.0 + 32.0;
        break;
      }                                                                  
    }
    rawValue = 0.0;
  }

  return prevTemp;
}

void setup() 
{
  Serial.begin(9600);
  
  pinMode(UNUSED_CNTRL_PIN, OUTPUT);
  digitalWrite(UNUSED_CNTRL_PIN, LOW);

  pinMode(FAN_CNTRL_PIN, OUTPUT);
  digitalWrite(FAN_CNTRL_PIN, LOW);

  pinMode(HUMIDITY_CNTRL_PIN, OUTPUT);
  digitalWrite(HUMIDITY_CNTRL_PIN, LOW);

  pinMode(TEMP_CNTRL_PIN, OUTPUT);
  digitalWrite(TEMP_CNTRL_PIN, LOW);

  pinMode(LIGHTS_PIN, OUTPUT);
  digitalWrite(LIGHTS_PIN, LOW);
  
  dht1.begin();
  dht2.begin();

  TFT_BL_ON;                                          //turn on the background light 
    
  Tft.TFTinit();                                      //init TFT library
  
  TCCR2A = 0x81;
  TCCR2B = 0x04;
  OCR2A = 127;

  if(rtc.begin())
  {
    if (rtc.lostPower()) 
    {
      // following line sets the RTC to the date & time this sketch was compiled
      rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
      // This line sets the RTC with an explicit date & time, for example to set
      // January 21, 2014 at 3am you would call:
      // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    }
  }

  uint8_t eeprom_data[3];
  //READ!!!!*********************************
  delay(10);
  Wire.beginTransmission(AT24C32_Address);
  if (Wire.endTransmission() == 0)
  {
    Wire.beginTransmission(AT24C32_Address);
    Wire.write(0x00);
    Wire.write(0x00);
    if(Wire.endTransmission() == 0)
    {
      int r = 0;
      Wire.requestFrom(AT24C32_Address, 3);
      while (Wire.available() > 0 && r < 3) 
      {
        eeprom_data[r] = (byte)Wire.read();
        r++;
      }
    }
  }

  if(eeprom_data[0] != 0xA1)
  {
    Desired_Temp = 80;
    Desired_Humidity = 70;
    write_desired_values_to_eeprom();
  }
  else
  {
    Desired_Temp = eeprom_data[1];
    Desired_Humidity = eeprom_data[2];
  }

  draw_main();
}

void write_desired_values_to_eeprom()
{
  uint8_t eeprom_data[3];
  eeprom_data[0] = 0xA1;
  eeprom_data[1] = Desired_Temp;
  eeprom_data[2] = Desired_Humidity;
  //WRITE!!!!*******************************
  Wire.beginTransmission(AT24C32_Address);
  if (Wire.endTransmission() == 0)
  {
    Wire.beginTransmission(AT24C32_Address);
    Wire.write(0x00);
    Wire.write(0x00);
    for(uint8_t i = 0; i < 3; i++)      //Write 3 data bytes
    {
      Wire.write(eeprom_data[i]);
    }
    Wire.endTransmission();
  }
}

#define HOUR_ON (8)

bool dateTimeChanged = false;

void loop() 
{
  Point p = ts.getPoint();
  float rock_temp = get_rock_temp();

  if (p.z > __PRESSURE)
  {
    p.x = map(p.x, TS_MINX, TS_MAXX, 0, 240);
    p.y = map(p.y, TS_MINY, TS_MAXY, 0, 320);

    if(state == 0)
    {
      if(p.x > 170 && p.x < 240)
      {
        if(p.y > 100 && p.y < 159)
        {
          draw_set_temp();
        }
        else if(p.y > 159 && p.y < 220)
        {
          draw_set_hum();
        }      
      }
      else if(p.x > 10 && p.x < 230 && p.y > 10 && p.y < 79)
      {
        dateTimeChanged = false;
        draw_set_time();
      }
    }
    else if(state == 1)
    {
      if((p.x > 20 && p.x < 220) && (p.y > 260 && p.y < 300))
      {
        if(dateTimeChanged)
        {
          rtc.adjust(prev_time);
        }
        draw_main();
      }
      else if((p.x > 20 && p.x < 70) && (p.y > 5 && p.y < 35))
      {
        if(prev_time.month() < 12)
        {
          prev_time = DateTime(prev_time.year(), prev_time.month() + 1, prev_time.day(), prev_time.hour(), prev_time.minute(), prev_time.second());
          Tft.fillRectangle(20, 43, 50, 30, BLUE);
          Tft.drawNumber(prev_time.month(), 20, 43, 4, WHITE);
          dateTimeChanged = true;
        }
      }
      else if((p.x > 94 && p.x < 144) && (p.y > 5 && p.y < 35))
      {
        if(prev_time.day() < 31)
        {
          prev_time = DateTime(prev_time.year(), prev_time.month(), prev_time.day() + 1, prev_time.hour(), prev_time.minute(), prev_time.second());
          Tft.fillRectangle(94, 43, 50, 30, BLUE);
          Tft.drawNumber(prev_time.day(), 94, 43, 4, WHITE);
          dateTimeChanged = true;
        }
      }
      else if((p.x > 164 && p.x < 214) && (p.y > 5 && p.y < 35))
      {
        prev_time = DateTime(prev_time.year() + 1, prev_time.month(), prev_time.day(), prev_time.hour(), prev_time.minute(), prev_time.second());
        Tft.fillRectangle(164, 43, 50, 30, BLUE);
        Tft.drawNumber(prev_time.year() - 2000, 164, 43, 4, WHITE);
        dateTimeChanged = true;
      }
      else if((p.x > 20 && p.x < 70) && (p.y > 80 && p.y < 110))
      {
        if(prev_time.month() > 1)
        {
          prev_time = DateTime(prev_time.year(), prev_time.month() - 1, prev_time.day(), prev_time.hour(), prev_time.minute(), prev_time.second());
          Tft.fillRectangle(20, 43, 50, 30, BLUE);
          Tft.drawNumber(prev_time.month(), 20, 43, 4, WHITE);
          dateTimeChanged = true;
        }
      }
      else if((p.x > 94 && p.x < 144) && (p.y > 80 && p.y < 110))
      {
        if(prev_time.day() > 1)
        {
          prev_time = DateTime(prev_time.year(), prev_time.month(), prev_time.day() - 1, prev_time.hour(), prev_time.minute(), prev_time.second());
          Tft.fillRectangle(94, 43, 50, 30, BLUE);
          Tft.drawNumber(prev_time.day(), 94, 43, 4, WHITE);
          dateTimeChanged = true;
        }
      }
      else if((p.x > 164 && p.x < 214) && (p.y > 80 && p.y < 110))
      {
        prev_time = DateTime(prev_time.year() - 1, prev_time.month(), prev_time.day(), prev_time.hour(), prev_time.minute(), prev_time.second());
        Tft.fillRectangle(164, 43, 50, 30, BLUE);
        Tft.drawNumber(prev_time.year() - 2000, 164, 43, 4, WHITE);
        dateTimeChanged = true;
      }
      else if((p.x > 20 && p.x < 70) && (p.y > 128 && p.y < 158))
      {
        if(prev_time.hour() < 23)
        {
          prev_time = DateTime(prev_time.year(), prev_time.month(), prev_time.day(), prev_time.hour() + 1, prev_time.minute(), prev_time.second());
          Tft.fillRectangle(20, 165, 50, 30, BLUE);
          Tft.drawNumber(prev_time.hour(), 20, 165, 4, WHITE);
          dateTimeChanged = true;
        }
      }
      else if((p.x > 94 && p.x < 144) && (p.y > 128 && p.y < 158))
      {
        if(prev_time.minute() < 59)
        {
          prev_time = DateTime(prev_time.year(), prev_time.month(), prev_time.day(), prev_time.hour(), prev_time.minute() + 1, prev_time.second());
          Tft.fillRectangle(94, 165, 50, 30, BLUE);
          Tft.drawNumber(prev_time.minute(), 94, 165, 4, WHITE);
          dateTimeChanged = true;
        }
      }
      else if((p.x > 164 && p.x < 214) && (p.y > 128 && p.y < 158))
      {
        if(prev_time.second() < 59)
        {
          prev_time = DateTime(prev_time.year(), prev_time.month(), prev_time.day(), prev_time.hour(), prev_time.minute(), prev_time.second() + 1);
          Tft.fillRectangle(164, 165, 50, 30, BLUE);
          Tft.drawNumber(prev_time.second(), 164, 165, 4, WHITE);
          dateTimeChanged = true;
        }
      }
      else if((p.x > 20 && p.x < 70) && (p.y > 200 && p.y < 230))
      {
        if(prev_time.hour() > 0)
        {
          prev_time = DateTime(prev_time.year(), prev_time.month(), prev_time.day(), prev_time.hour() - 1, prev_time.minute(), prev_time.second());
          Tft.fillRectangle(20, 165, 50, 30, BLUE);
          Tft.drawNumber(prev_time.hour(), 20, 165, 4, WHITE);
          dateTimeChanged = true;
        }
      }
      else if((p.x > 94 && p.x < 144) && (p.y > 200 && p.y < 230))
      {
        if(prev_time.minute() > 0)
        {
          prev_time = DateTime(prev_time.year(), prev_time.month(), prev_time.day(), prev_time.hour(), prev_time.minute() - 1, prev_time.second());
          Tft.fillRectangle(94, 165, 50, 30, BLUE);
          Tft.drawNumber(prev_time.minute(), 94, 165, 4, WHITE);
          dateTimeChanged = true;
        }
      }
      else if((p.x > 164 && p.x < 214) && (p.y > 200 && p.y < 230))
      {
        if(prev_time.second() > 0)
        {
          prev_time = DateTime(prev_time.year(), prev_time.month(), prev_time.day(), prev_time.hour(), prev_time.minute(), prev_time.second() - 1);
          Tft.fillRectangle(164, 165, 50, 30, BLUE);
          Tft.drawNumber(prev_time.second(), 164, 165, 4, WHITE);
          dateTimeChanged = true;
        }
      }
    }
    else if(state == 2)
    {
      if((p.x > 20 && p.x < 220) && (p.y > 260 && p.y < 300))
      {
        write_desired_values_to_eeprom();
        draw_main();
      }
      else if(p.x > 150 && p.x < 215)
      {
        if(p.y > 105 && p.y < 145)
        {
          Desired_Temp++;
          Tft.fillRectangle(25, 145, 75, 48, BLACK);
          Tft.drawNumber(Desired_Temp, 25, 145, 6, WHITE);
        }
        else if(p.y > 185 && p.y < 225)
        {
          Desired_Temp--;
          Tft.fillRectangle(25, 145, 75, 48, BLACK);
          Tft.drawNumber(Desired_Temp, 25, 145, 6, WHITE);
        }
      }
    }
    else if(state == 3)
    {
      if((p.x > 20 && p.x < 220) && (p.y > 260 && p.y < 300))
      {
        write_desired_values_to_eeprom();
        draw_main();
      }
      else if(p.x > 150 && p.x < 215)
      {
        if(p.y > 105 && p.y < 145)
        {
          Desired_Humidity++;
          Tft.fillRectangle(25, 145, 75, 48, BLACK);
          Tft.drawNumber(Desired_Humidity, 25, 145, 6, WHITE);
        }
        else if(p.y > 185 && p.y < 225)
        {
          Desired_Humidity--;
          Tft.fillRectangle(25, 145, 75, 48, BLACK);
          Tft.drawNumber(Desired_Humidity, 25, 145, 6, WHITE);
        }
      }
    }
  }

  if(loopCount >= 30)
  {
    DateTime now = rtc.now();
    
    // Reading temperature or humidity takes about 250 milliseconds!
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    Current_Humidity_1 = dht1.readHumidity();
    // Read temperature as Celsius (the default)
    Current_Temp_1 = dht1.readTemperature(true);

    Current_Humidity_2 = dht2.readHumidity();
    // Read temperature as Celsius (the default)
    Current_Temp_2 = dht2.readTemperature(true);

    if(Current_Humidity_1 <= 0.0 && Current_Temp_1 <= 0.0)
    {
      dht1.begin();
    }

    if(Current_Humidity_2 <= 0.0 && Current_Temp_2 <= 0.0)
    {
      dht2.begin();
    }

    if(state == 0)
    {
      //Display Temp and Humidity
      Tft.fillRectangle(2, 124, 75, 30, BLACK);
      Tft.drawFloat(Current_Temp_1, 1, 2, 124, 3, CYAN);
      Tft.fillRectangle(83, 124, 75, 30, BLACK);
      Tft.drawFloat(Current_Temp_2, 1, 83, 124, 3, CYAN);
      Tft.fillRectangle(2, 206, 75, 30, BLACK);
      Tft.drawFloat(Current_Humidity_1, 1, 2, 206, 3, CYAN);
      Tft.fillRectangle(83, 206, 75, 30, BLACK);
      Tft.drawFloat(Current_Humidity_2, 1, 83, 206, 3, CYAN);

      //Display Current Date and Time.
      if(prev_time.month() != now.month())
      {
        Tft.fillRectangle(20, 10, 50, 30, BLACK);
        Tft.drawNumber(now.month(), 20, 10, 4, WHITE);
      }

      if(prev_time.day() != now.day())
      {
        Tft.fillRectangle(92, 10, 50, 30, BLACK);
        Tft.drawNumber(now.day(), 92, 10, 4, WHITE);
      }

      if(prev_time.year() != now.year())
      {
        Tft.fillRectangle(164, 10, 50, 30, BLACK);
        Tft.drawNumber(now.year() - 2000, 164, 10, 4, WHITE);
      }

      if(prev_time.hour() != now.hour())
      {
        Tft.fillRectangle(20, 45, 50, 30, BLACK);
        Tft.drawNumber(now.hour(), 20, 45, 4, WHITE);
      }

      if(prev_time.minute() != now.minute())
      {
        Tft.fillRectangle(92, 45, 50, 30, BLACK);
        Tft.drawNumber(now.minute(), 92, 45, 4, WHITE);
      }
      
      Tft.fillRectangle(164, 45, 50, 30, BLACK);
      Tft.drawNumber(now.second(), 164, 45, 4, WHITE);

      Tft.fillRectangle(70, 267, 150, 30, BLACK);
      Tft.drawFloat(rock_temp, 1, 70, 267, 4, CYAN);

      prev_time = now;
    }

    /***************************************************/
    //Control Humidifier here.
    /***************************************************/
    if(now.minute() >= 0 && now.minute() <= 5)
    {
      digitalWrite(HUMIDITY_CNTRL_PIN, HIGH);
    }
    else if(now.minute() >= 30 && now.minute() <= 35)
    {
      digitalWrite(HUMIDITY_CNTRL_PIN, HIGH);
    }
    else
    {
      digitalWrite(HUMIDITY_CNTRL_PIN, LOW);
    }
    /***************************************************/

    /***************************************************/
    //Control FAN here.
    /***************************************************/
    //digitalWrite(FAN_CNTRL_PIN, HIGH);
    //digitalWrite(FAN_CNTRL_PIN, LOW);
    /***************************************************/

    /***************************************************/
    //Control Lights here.
    /***************************************************/
    if(now.hour() < HOUR_ON)
    { //Lights off all night.
      OCR2A = 0;
    }
    else if(now.hour() == HOUR_ON)
    {
      if(now.minute() >= 30)
      { //Lights 100%
        OCR2A = 255;
      }
      else
      { //Lights go from 0% to 100% over 30 min.
        float n = (float)now.minute() * 256.0 / 30.0;
        OCR2A = (uint8_t)n;
      }
    }
    else if(now.hour() > HOUR_ON && now.hour() < (HOUR_ON + 12))
    { //Lights on all day at 100%
      OCR2A = 255;
    }
    else if(now.hour() == (HOUR_ON + 12))
    {
      if(now.minute() >= 30)
      { // Lights Off
        OCR2A = 0;
      }
      else
      { //Lights go from 100% to 0% over 30 min.
        float n = (30.0 - (float)now.minute()) * 256.0 / 30.0 - 1.0;
        OCR2A = (uint8_t)n;
      }
    }
    else if(now.hour() > (HOUR_ON + 12))
    { //Lights off all night.
      OCR2A = 0;
    }

    loopCount = 0;
  }
  else
  {
    loopCount++;
  }

  control_loop(rock_temp);
  
  delay(100);
}
