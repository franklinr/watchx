// Good version without Serial

#include <Adafruit_SSD1306.h>
#include <Fonts/Org_01.h>
#include <MPU6050.h>
#include "RTClib.h"
#include <SparkFun_MAG3110.h>
#include <Adafruit_BMP280.h>

#define BMP_SCK 13
#define BMP_MISO 12
#define BMP_MOSI 11
#define BMP_CS 10

Adafruit_BMP280 bme; // I2C
long recordRate[] = {1L, 100L, 500L, 1000L, 5000L, 30000L, 60000L, 1800000L, 3600000L};
int rate = 0;
//int rate = 7; // 30 minutes
#define MAXRATE 9;

DateTime startTime;
DateTime recentsave;
RTC_DS3231 rtc;
MAG3110 mag = MAG3110(); //Instantiate MAG3110

#define HISTMAX 100
#define TIMEMEM HISTMAX+10 // EEPROM memory location for most recent time info

// to save the data in the EEPROM uncomment this line
// #define USEEEPROM

#ifdef USEEEPROM
#include <EEPROM.h>

#define UNIT sizeof(int)

void initData() {
  // initialize EEPROM if using for the first time
  /*  for (int i = 0 ; i < HISTMAX ; i++) {
      EEPROM.put(i * sizeof(int), 0);
    }*/
}

int readData(int i) {
  int py;
  EEPROM.get(i * UNIT, py);
  return (py);
}

void writeData(int i, int d) {
  EEPROM.put(i * sizeof(int), d);
}



#else
int hist[TIMEMEM + 20];
#define UNIT 0

void initData() {
  for (int i = 0; i < HISTMAX; i++) {
    hist[i] = 0;
  }
}

int readData(int i) {
  return (hist[i]);
}

void writeData(int i, int d) {
  hist[i] = d;
}

#endif


// Uncomment this block to use hardware SPI
#define OLED_DC     A3
#define OLED_CS     A5
#define OLED_RESET  A4
Adafruit_SSD1306 display(OLED_DC, OLED_RESET, OLED_CS);

#define BUTTON1 8
#define BUTTON2 11
#define BUTTON3 10
#define BUZZER 9
#define BATTERYINPUT 4
#define BATTERYOUT A11

#define SCREENX 128
#define SCREENY 64

MPU6050 mpu;

int state = 0;
byte lastButtonState = 0;
byte showScreen = 0;
String label = "NONE";

int mpupos = 0;
int maxy = 13;
int miny = -24;


void writeData(int d) {
  mpupos = readData(HISTMAX + 1);
  mpupos = (mpupos + 1) % HISTMAX;
  writeData(HISTMAX + 1, mpupos);
  writeData(mpupos, d);
  saveTimeRecord();
}

int getMPU(int pr) {
  Vector normAccel = mpu.readNormalizeAccel();
  if (pr == 0) {
    int pitch = -(atan2(normAccel.XAxis, sqrt(normAccel.YAxis * normAccel.YAxis + normAccel.ZAxis * normAccel.ZAxis)) * 180.0) / M_PI;
    return (pitch);
  } else {
    int roll = (atan2(normAccel.YAxis, normAccel.ZAxis) * 180.0) / M_PI;
    return (roll);
  }
  return (0);
}


int getMAG() {
  int x, y, z;
  if (!mag.isCalibrated()) //If we're not calibrated
  {
    if (!mag.isCalibrating()) //And we're not currently calibrating
    {
      // Serial.println("Entering calibration mode");
      mag.enterCalMode(); //This sets the output data rate to the highest possible and puts the mag sensor in active mode
    }
    else
    {

      //Must call every loop while calibrating to collect calibration data
      //This will automatically exit calibration
      //You can terminate calibration early by calling mag.exitCalMode();
      mag.calibrate();
    }
  }
  else //We are calibrated
  {
    int angle =  mag.readHeading();
    return (angle);
  }
  return (0);
}

void setup() {
  initRTC();

  activateMPU();
  activateMAG();
  bme.begin();

  display.begin(SSD1306_SWITCHCAPVCC);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setFont(&Org_01);
  display.clearDisplay();
  display.display();

  pinMode(BUTTON3, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  pinMode(BUTTON1, INPUT_PULLUP);

  initData();

  turnOnShowData();
  state = 0;
  incrementState();
  //  getWriteBattery();
  //  getWriteBattery();
}

#define GRAPHY0 SCREENY - 3
#define GRAPHHEIGHT (SCREENY - 7)


int mapGraph(int sensorval) {
  int valrange = maxy - miny;
  int mapval = GRAPHY0 - (sensorval - miny) * GRAPHHEIGHT / valrange;
  return (mapval);
}

void drawStat(int ypos, String lab) {

  display.drawLine(1, 0, 1, SCREENY, 1); // yaxis
  display.setCursor(3, 6);
  display.print(maxy);
  // display.drawLine(1, 4, 4, 4, 1);
  display.setCursor(3, SCREENY - 1 );
  display.print(miny);
  // display.drawLine(1, SCREENY -1, 4, SCREENY -1, 1);

  // mpupos=(mpupos+1)%HISTMAX;
  // hist[mpupos]=cur;
  int py = 0;
  //  EEPROM.get(mpupos*sizeof(int), py);
  py = readData(mpupos);
  display.setCursor(5, SCREENY - 12);
  display.print(label);
  display.print(F(": "));
  display.print(py);

  // display.drawLine(40, mapGraph(0), SCREENX, mapGraph(0), 1);
  ypos = ypos + 15;

  for (int i = 1; i < HISTMAX; i++) {
    int d = mpupos - i;
    if (d < 0) {
      d = d + HISTMAX;
    }
    int pynow = 0;
    //    EEPROM.get(d*sizeof(int), pynow);
    pynow = readData(d);
    if (pynow > maxy) {
      maxy = pynow;
    }
    if (pynow < miny) {
      miny = pynow;
    }
    if (HISTMAX - i > 10) {
      display.drawLine(HISTMAX - (i + 1) + 20, mapGraph(pynow), HISTMAX - i + 20, mapGraph(py), 1);
    }
    py = pynow;
  }
}

float readBattery() {
  digitalWrite(BATTERYINPUT, HIGH);
  delay(50);
  float voltage = analogRead(BATTERYOUT);
  voltage = (voltage / 1024) * 3.35;
  voltage = voltage / 0.5;
  delay(50);
  digitalWrite(BATTERYINPUT, LOW);
  return (voltage);
}


void showData() {
  display.ssd1306_command(SSD1306_DISPLAYON);
  display.clearDisplay();
  drawStat(SCREENY - 12, F("VOLT: "));
  display.setCursor(5, SCREENY - 18);
  display.print(F("RATE: "));
  display.print(recordRate[rate]);
  if (UNIT > 0) {
    display.print(F(" E"));
  } else {
    display.print(F(" R"));
  }
  if (IsUSBConnected()) {
    display.print(F(" U"));
  }
  display.setCursor(5, SCREENY - 24);
  int mintime = minStartMostRecent();
  if (mintime > -1) {
    display.print(F("MIN: "));
    display.print(mintime);
  }

  if (mag.isCalibrating()) {
    display.setCursor(5, 15);
    display.print(F("CALIBRATING"));
  }

  display.display();


}

int count = 0, lastcount = 1;

void turnOnShowData() {
  showScreen = 1;
  int pmin = miny;
  miny = maxy - 1;
  maxy = pmin;
  showData();
}

void incrementRate() {
  rate = (rate + 1) % MAXRATE;
  showScreen = 1;
  //showData();
}



void incrementState() {
  state = (state + 1) % 7;
  showScreen = 1;
  switch (state) {
    case 1:
      label = F("PITCH");
      break;
    case 2:
      label = F("ROLL");
      break;
    case 3:
      label = F("MAGN");
      break;
    case 4:
      label = F("TEMP");
      break;
    case 5:
      label = F("PRES");
      break;
    default:
      label = F("VOLT");
      break;
  }
  initData();
  showData();
}

bool usbConnected = true;

void loop() {
  if (usbConnected && !IsUSBConnected2()) {
    digitalWrite(LED_BUILTIN, HIGH);
    writeDateTimeStart();
    sleepMillis(1000);
    digitalWrite(LED_BUILTIN, LOW);
  }
  usbConnected = IsUSBConnected2();

  if (showScreen == 1) {
    showData();
  }

  if (awakeRecTimeMilli() > recordRate[rate]) {
    setRecTime();
    int dat = 0;
    switch (state) {
      case 1:
        dat = getMPU(0);
        break;
      case 2:
        dat = getMPU(1);
        break;
      case 3:
        dat = getMAG();
        break;
      case 4:
        dat = bme.readTemperature();
        break;
      case 5:
        //       dat = bme.readAltitude(1013.25);
        dat = bme.readPressure();
        break;
      /*    case 6:

          break;*/
      default:
        dat = readBattery() * 100;
        break;
    }
    writeData(dat);
  }

  if (awakeTimeSec() > 60) {
    display.ssd1306_command(SSD1306_DISPLAYOFF);
    setWakeTime();
    showScreen = 0;
  }

  if (recordRate[rate] > 2000) {
    sleepMillis(2000);
  }
  count = count + 1;
}
