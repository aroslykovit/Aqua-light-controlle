#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <avr/eeprom.h>
#define PCF8563address 0x51 // Адрес устройства по умолчанию 
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <util/delay.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for SSD1306 display connected using software SPI (default case):
#define OLED_MOSI   9
#define OLED_CLK   10
#define OLED_DC    11
#define OLED_CS    12
#define OLED_RESET 13
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT,
                         OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);

long unsigned int lastTime = 0;
int millisPress = 0;
volatile int counter = 0;   // счётчик
volatile bool encFlag = 0;  // флаг поворота
byte mainMenuCnt = 0;
byte numOfMenu = 0;
bool buttonPress = 0;
bool longPress = 0;
bool lastBtnState = 0;
bool pinState = 0;
bool turnDisplayFlag = 1;


byte startHour = 8;
byte endHour = 17;
byte beginMinuite = 0;
byte endMinuite = 0;
byte second, minute, hour, dayOfWeek, dayOfMonth, month, year;

// функция перевода из двоично - десятичной системы в десятичную.
byte bcdToDec(byte value)
{
  return ((value / 16) * 10 + value % 16);
}
// И обратно
byte decToBcd(byte value) {
  return (value / 10 * 16 + value % 10);
}

// функция установки времени и даты в PCF8563
void setPCF8563()
{
  Wire.beginTransmission(PCF8563address);
  Wire.write(0x02);
  Wire.write(decToBcd(second));
  Wire.write(decToBcd(minute));
  Wire.write(decToBcd(hour));
  Wire.write(decToBcd(dayOfMonth));
  Wire.write(decToBcd(dayOfWeek));
  Wire.write(decToBcd(month));
  Wire.write(decToBcd(year));
  Wire.endTransmission();
}

// функция считывания времени и даты из PCF8563
void readPCF8563()
{
  Wire.beginTransmission(PCF8563address);
  Wire.write(0x02);
  Wire.endTransmission();
  Wire.requestFrom(PCF8563address, 3);
  second   = bcdToDec(Wire.read() & B01111111); // удаление ненужных бит из данных
  minute   = bcdToDec(Wire.read() & B01111111);
  hour    = bcdToDec(Wire.read() & B00111111);
  dayOfMonth = bcdToDec(Wire.read() & B00111111);
  dayOfWeek = bcdToDec(Wire.read() & B00000111);
  month   = bcdToDec(Wire.read() & B00011111);
  year    = bcdToDec(Wire.read());
}

void UpdatePinState() {
  if (hour > startHour && hour < endHour) {
    pinState = 1;
  }
  else {
    if (hour == startHour && minute >= beginMinuite) {
      pinState = 1;
    }
    else {
      if (hour == endHour && minute < endMinuite) {
        pinState = 1;
      }
      else {
        pinState = 0;
      }
    }
  }
  digitalWrite(6, pinState);
}

void setup() {
  /*Wire.begin();

    pinMode(6, OUTPUT);
    pinMode(5, INPUT_PULLUP);
    pinMode(4, INPUT_PULLUP);
    pinMode(2, INPUT_PULLUP);
    pinMode(3, INPUT_PULLUP);
    attachInterrupt(0, encIsr, CHANGE);
    attachInterrupt(1, encIsr, CHANGE);

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if (!display.begin(SSD1306_SWITCHCAPVCC)) {
    for (;;); // Don't proceed, loop forever
    }

    startHour = eeprom_read_byte(0);
    beginMinuite = eeprom_read_byte(1);
    endHour = eeprom_read_byte(2);
    endMinuite = eeprom_read_byte(3);

    readPCF8563();


    // Show initial display buffer contents on the screen --
    // the library initializes this with an Adafruit splash screen.
    /*display.clearDisplay();

    display.setTextSize(1);             // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE);        // Draw white text
    display.setCursor(0,0);             // Start at top-left corner
    display.println(F("Hello, world!"));
    display.display();
    delay(2000); // Pause for 2 seconds*/

  // Clear the buffer
  /*display.clearDisplay();
    updateDisplay();

    UpdatePinState();*/

}

void loop() {

  pinMode(5, INPUT_PULLUP);
  pinMode(6, OUTPUT);


  while (!digitalRead(5)) {

    if (turnDisplayFlag) {
      turnDisplayFlag = 0;
      Wire.begin();

      pinMode(4, INPUT_PULLUP);
      pinMode(2, INPUT_PULLUP);
      pinMode(3, INPUT_PULLUP);
      attachInterrupt(0, encIsr, CHANGE);
      attachInterrupt(1, encIsr, CHANGE);
      startHour = eeprom_read_byte(0);
      beginMinuite = eeprom_read_byte(1);
      endHour = eeprom_read_byte(2);
      endMinuite = eeprom_read_byte(3);

      readPCF8563();
      UpdatePinState();

      // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
      if (!display.begin(SSD1306_SWITCHCAPVCC)) {
        for (;;); // Don't proceed, loop forever
      }

      // Clear the buffer
      display.clearDisplay();
      updateDisplay();
    }

    if (!digitalRead(4)) {
      buttonPress = 1;
      lastTime = millis();
    }

    while (!digitalRead(4)) {
      if (millis() - lastTime >= 2000) {
        longPress = 1;
      }
    }

    if (buttonPress) {
      buttonPress = 0;
      if (numOfMenu == 0) {
        if (mainMenuCnt == 0) {
          numOfMenu = 1;
          mainMenuCnt = 0;
          counter = startHour;
        }
        else {
          if (mainMenuCnt == 1) {
            numOfMenu = 2;
            mainMenuCnt = 0;
            counter = endHour;
          }
          else {
            if (mainMenuCnt == 2) {
              numOfMenu = 3;
              mainMenuCnt = 0;
              counter = hour;
              readPCF8563();
            }
          }
        }
      }
      else {
        if (numOfMenu == 1) {
          if (!longPress) {
            if (mainMenuCnt == 0) {
              mainMenuCnt = 1;
              counter = beginMinuite;
            }
            else {
              if (mainMenuCnt == 1) {
                mainMenuCnt = 0;
                counter = startHour;
              }
            }
          }
          else {
            eeprom_write_byte(0, startHour);
            eeprom_write_byte(1, beginMinuite);
            numOfMenu = 0;
            counter = 0;
            mainMenuCnt = 0;
          }
        }
        else {
          if (numOfMenu == 2) {
            if (!longPress) {
              if (mainMenuCnt == 0) {
                mainMenuCnt = 1;
                counter = endMinuite;
              }
              else {
                if (mainMenuCnt == 1) {
                  mainMenuCnt = 0;
                  counter = endHour;
                }
              }
            }
            else {
              eeprom_write_byte(2, endHour);
              eeprom_write_byte(3, endMinuite);
              numOfMenu = 0;
              counter = 0;
              mainMenuCnt = 0;
            }
          }
          else {
            if (numOfMenu == 3) {
              if (!longPress) {
                if (mainMenuCnt == 0) {
                  mainMenuCnt = 1;
                  counter = minute;
                }
                else {
                  if (mainMenuCnt == 1) {
                    mainMenuCnt = 0;
                    counter = hour;
                  }
                }
              }
              else {
                setPCF8563();
                numOfMenu = 0;
                counter = 0;
                mainMenuCnt = 0;
              }
            }
          }
        }
      }

      longPress = 0;
      updateDisplay();
    }

    if (encFlag) {
      lastTime = millis();
      encFlag = 0;
      if (numOfMenu == 0) {
        if (counter >= 2) {
          mainMenuCnt = 2;
          counter = 2;
        }
        else {
          if (counter <= 0) {
            mainMenuCnt = 0;
            counter = 0;
          }
          else {
            mainMenuCnt = counter;
          }
        }
      }
      else {
        if (numOfMenu == 1) {
          if (mainMenuCnt == 0) {
            if (counter > 24 || counter < 0) {
              counter = 0;
            }
            startHour = counter;
          }
          else {
            if (mainMenuCnt == 1) {
              if (counter > 60 || counter < 0) {
                counter = 0;
              }
              beginMinuite = counter;
            }
          }
        }

        else {
          if (numOfMenu == 2) {
            if (mainMenuCnt == 0) {
              if (counter > 24 || counter < 0) {
                counter = 0;
              }
              endHour = counter;
            }
            else {
              if (mainMenuCnt == 1) {
                if (counter > 60 || counter < 0) {
                  counter = 0;
                }
                endMinuite = counter;
              }
            }
          }
          else {
            if (numOfMenu == 3) {
              if (mainMenuCnt == 0) {
                if (counter > 24 || counter < 0) {
                  counter = 0;
                }
                hour = counter;
              }
              else {
                if (mainMenuCnt == 1) {
                  if (counter > 60 || counter < 0) {
                    counter = 0;
                  }
                  minute = counter;
                }
              }
            }
          }
        }
      }
      updateDisplay();
    }
  }
  if (!turnDisplayFlag) {
    turnDisplayFlag = 1;
    Wire.begin();

    pinMode(4, INPUT_PULLUP);
    pinMode(2, INPUT_PULLUP);
    pinMode(3, INPUT_PULLUP);
    startHour = eeprom_read_byte(0);
    beginMinuite = eeprom_read_byte(1);
    endHour = eeprom_read_byte(2);
    endMinuite = eeprom_read_byte(3);

    readPCF8563();
    UpdatePinState();

    // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
    if (!display.begin(SSD1306_SWITCHCAPVCC)) {
      for (;;); // Don't proceed, loop forever
    }

    // Clear the buffer
    display.clearDisplay();
    display.display();
  }
  else {
    Wire.begin();
    startHour = eeprom_read_byte(0);
    beginMinuite = eeprom_read_byte(1);
    endHour = eeprom_read_byte(2);
    endMinuite = eeprom_read_byte(3);

    readPCF8563();
    UpdatePinState();
  }
  /*wdt_enable(WDTO_1S); //Задаем интервал сторожевого таймера (8с)
  WDTCSR |= (1 << WDIE); //Устанавливаем бит WDIE регистра WDTCSR для разрешения прерываний от сторожевого таймера
  set_sleep_mode(SLEEP_MODE_PWR_DOWN); //Устанавливаем интересующий нас режим
  sleep_mode(); // Переводим МК в спящий режим*/

}

/*ISR (WDT_vect) {
  wdt_disable();
}*/

void updateDisplay() {
  if (numOfMenu == 0) {
    if (mainMenuCnt == 0) {
      display.clearDisplay();
      display.setTextSize(2);             // Normal 1:1 pixel scale
      display.setTextColor(SSD1306_WHITE);        // Draw white text
      display.setCursor(0, 0);            // Start at top-left corner
      display.println(F(">On Time"));
      display.println(F("Off Time"));
      display.println(F("Time"));
    }
    else {
      if (mainMenuCnt == 1) {
        display.clearDisplay();
        display.setTextSize(2);             // Normal 1:1 pixel scale
        display.setTextColor(SSD1306_WHITE);        // Draw white text
        display.setCursor(0, 0);            // Start at top-left corner
        display.println(F("On Time"));
        display.println(F(">Off Time"));
        display.println(F("Time"));
      }
      else {
        if (mainMenuCnt == 2) {
          display.clearDisplay();
          display.setTextSize(2);             // Normal 1:1 pixel scale
          display.setTextColor(SSD1306_WHITE);        // Draw white text
          display.setCursor(0, 0);            // Start at top-left corner
          display.println(F("On Time"));
          display.println(F("Off Time"));
          display.println(F(">Time"));
        }
      }
    }

  }
  if (numOfMenu == 1) {
    display.clearDisplay();
    display.setTextSize(4);             // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE);        // Draw white text
    display.setCursor(0, 0);            // Start at top-left corner
    display.print(startHour);
    display.print(F(":"));
    display.print(beginMinuite);
  }
  if (numOfMenu == 2) {
    display.clearDisplay();
    display.setTextSize(4);             // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE);        // Draw white text
    display.setCursor(0, 0);            // Start at top-left corner
    display.print(endHour);
    display.print(F(":"));
    display.print(endMinuite);
  }
  if (numOfMenu == 3) {
    display.clearDisplay();
    display.setTextSize(4);             // Normal 1:1 pixel scale
    display.setTextColor(SSD1306_WHITE);        // Draw white text
    display.setCursor(0, 0);            // Start at top-left corner
    display.print(hour);
    display.print(F(":"));
    display.print(minute);
  }
  display.display();
}

volatile byte reset = 0, last = 0;
void encIsr() {
  byte state = (PIND & 0b1100) >> 2;  // D2 + D3
  if (reset && state == 0b11) {
    int prevCount = counter;
    if (last == 0b10) counter++;
    else if (last == 0b01) counter--;
    if (prevCount != counter) encFlag = 1;
    reset = 0;
  }
  if (!state) reset = 1;
  last = state;
}
