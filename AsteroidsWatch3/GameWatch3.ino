
// EnableInterrupt Simple example sketch. Demonstrates operation on a single pin of your choice.
// See https://github.com/GreyGnome/EnableInterrupt and the README.md for more information.
#include <Adafruit_SSD1306.h>
#include "RTClib.h"
#include <MPU6050.h>

// Uncomment this block to use hardware SPI
#define OLED_DC     A3
#define OLED_CS     A5
#define OLED_RESET  A4

#define BUTTON1 8
#define BUTTON2 11
#define BUTTON3 10
#define VOLTAGEDIV 0.5
#define BATTERYENERGY 4
#define BATTERYINPUT A11
#define PI8 PI/8
#define PI4 PI/4
#define PI2 PI/2
#define TWOPI PI*2

Adafruit_SSD1306 display(OLED_DC, OLED_RESET, OLED_CS);
RTC_DS3231 rtc;
MPU6050 mpu;

#define SCREENX 128
#define SCREENY 64
#define MIDSCREENX SCREENX/2
#define MIDSCREENY SCREENY/2

int count = 0;
byte state = 0;
byte progMode = 1;

DateTime usbtime, wakeRefTime;
DateTime now;
float batteryLevel = 0;
float usbbatteryLevel = 0;

void setup() {
  rtc.begin();
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  randomSeed(analogRead(0));

  display.begin(SSD1306_SWITCHCAPVCC);
  display.setTextSize(1);
  display.setTextColor(WHITE);

  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  pinMode(BUTTON3, INPUT_PULLUP);

  // turn off accelerometer
  // mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G);
  // mpu.setSleepEnabled(true);
  while (!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G))
  {
    //  Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    delay(500);
  }

  // turnOffBluetooth();
  usbbatteryLevel = getBatteryPercent();
  batteryLevel = usbbatteryLevel;
  now  = rtc.now();
  usbtime  = now;
  wakeRefTime = now;
}


float getBatteryPercent() {
  digitalWrite(BATTERYENERGY, HIGH);
  delay(50);
  float batteryLevelHistory = batteryLevel;
  float voltage = analogRead(BATTERYINPUT);
  voltage = (voltage / 1024) * 3.35;
  voltage = voltage / VOLTAGEDIV;
  delay(50);
  digitalWrite(BATTERYENERGY, LOW);
  batteryLevel = (voltage - 3.38) / 0.0084;
  batteryLevel = (batteryLevel + batteryLevelHistory) / 2;

  return (batteryLevel);
}

void showBattery() {
  display.setCursor(0, 20);
  display.print(F("Bat:"));
  display.print((int) batteryLevel);
  display.print(F("%"));
  if (progMode > 0) {
    display.print(F(" C"));
  } else {
    //uint32_t millissinceusb = millis() - usbmillis;
    TimeSpan rtcusb = rtc.now() - usbtime;
    display.setCursor(0, 30);
    display.print(F("SinceUSB:"));
    int32_t totalsec =  rtcusb.totalseconds();
    display.print(totalsec);

    display.setCursor(0, 40);
    display.print(F("BatChange:"));
    float batdiff = (usbbatteryLevel - batteryLevel);
    display.print(batdiff);

    if (batdiff > 0 && totalsec > 0) {
      float minleft = batteryLevel * totalsec / (batdiff * 60);
      display.setCursor(0, 50);
      display.print(F("EstBatLife:"));
      display.print((int) minleft);
    }
  }
}

void showTime(int colon) {
  display.setCursor(0, 0);
  display.print(now.hour());
  if (colon > 0) {
    display.print(F(":"));
  }
  int m = now.minute();
  if (m < 10) {
    display.print(0);
  }
  display.print(m);
}

void showDate() {
  display.setCursor(0, 10);
  display.print(now.year());
  display.print(F("/"));
  display.print(now.month());
  display.print(F("/"));
  display.print(now.day());
}


int lastButtonState = 0, lastButtonState2 = 0;  // previous state of the button

void loop() {
  int buttonState = digitalRead(BUTTON3);

  if (buttonState == LOW && lastButtonState == HIGH) {
    //&& buttonState != lastButtonState
    state = state + 1;
    count = 0;
    wakeRefTime = rtc.now();
    state = state % 4;
    if (state == 4 || state == 0) {
      resetInvaders();
    }
    if (state == 5) {
      resetAsteriods();
    }
    sleepMillis(50);
  }
  lastButtonState = buttonState;

  int buttonState2 = digitalRead(BUTTON2);
  if (buttonState2 == LOW && lastButtonState2 == HIGH) {
    if (state < 2) {
      state = state + 4;
    }
  }
  lastButtonState2 = buttonState2;

  now  = rtc.now();
  if (count % 100 == 0) {
    batteryLevel = getBatteryPercent();
  }
  Vector normAccel = mpu.readNormalizeAccel();
  int pitch = -(atan2(normAccel.XAxis, sqrt(normAccel.YAxis * normAccel.YAxis + normAccel.ZAxis * normAccel.ZAxis)) * 180.0) / M_PI;
  int roll = (atan2(normAccel.YAxis, normAccel.ZAxis) * 180.0) / M_PI;

  display.clearDisplay();

  switch (state) {
    case 4:
      showTime(0);
      drawInvadersGame(roll);
      display.display();
      wakeRefTime=rtc.now();
      break;
    case 5:
      showTime(0);
      drawAsteroidsGame(pitch);
      display.display();
      wakeRefTime=rtc.now();
      break;
    case 0:
      showTime(0);
      drawInvadersGame(-1000);
      display.display();
      break;
    case 1:
      showTime(0);
      drawAsteroidsGame(-1000);
      display.display();
      break;
    case 2:
      state = state + 1;
      //  drawJapanese();
      //   display.display();
      break;
    default:
      batteryLevel = getBatteryPercent();
      showTime(1);
      showDate();
      showBattery();
      sleepMillis(100);
      display.display();
      break;
  }

  setSleepStatusBasedonUSB();
  count++;
  count = count % 65534;


}

