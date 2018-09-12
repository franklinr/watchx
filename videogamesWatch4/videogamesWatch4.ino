#include <Adafruit_SSD1306.h>
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
MPU6050 mpu;

#define SCREENX 128
#define SCREENY 64
#define MIDSCREENX SCREENX/2
#define MIDSCREENY SCREENY/2

int count = 0;
byte state = 0;
byte progMode = 1;

float batteryLevel = 0;
float usbbatteryLevel = 0;

void initializeScreenSSD1306() {
  display.begin(SSD1306_SWITCHCAPVCC);
  display.setTextSize(1);
  display.setTextColor(WHITE);
}

void turnOffMPU6050() {
  mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G);
  mpu.setSleepEnabled(true);
}

void turnOnMPU6050() {
  while (!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G))
  {
    delay(500);
  }
}

void setup() {
  initRTC();

  randomSeed(analogRead(0)); // set random seed

  initializeScreenSSD1306();

  turnOffMPU6050();
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
  return (batteryLevel);
}


void showBattery() {
  display.setCursor(0, 8);
  display.print((int) batteryLevel);
}

byte lastButtonState = 0, lastButtonState2 = 0;  // previous state of the button

void loop() {
  if (count % 100 == 0) {
    setPresTime();
    batteryLevel = getBatteryPercent();
  }

  switch (state) {
    case 1:
      display.clearDisplay();
      showTime(1);
      //showBattery();
      showDate(); 
      drawInvadersGame(-1000);
      sleepControl();
      break;
    case 0:
      display.clearDisplay();
      showTime(1);
      showDate(); 
      drawAsteroidsGame(-1000);
      sleepControl();
      break;
    case 2:
      runEtchASketch();
      break;
    case 4:
      display.clearDisplay();
      drawInvadersGame(0);
      break;
    case 3:
      display.clearDisplay();
      drawAsteroidsGame(0);
      break;
  }

  count++;
  count = count % 32766;
}


