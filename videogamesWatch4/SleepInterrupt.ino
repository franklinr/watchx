#include <avr/sleep.h>
#include "RTClib.h"
#include "PinChangeInterrupt.h"
#include <avr/power.h>

RTC_DS3231 rtc;

#define PIN_INTERRUPT   PD1 // interrupt pin

DateTime usbtime, wakeRefTime, recTime, presTime;
int sleepState = 0;

void attachButtons() {
  attachPCINT(digitalPinToPCINT(BUTTON3), wakeScreen, FALLING);
  attachPCINT(digitalPinToPCINT(BUTTON2), incrState, FALLING);
}

void initRTC() {
  rtc.begin();
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  setWakeTime();
  setPresTime();

  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);
  pinMode(BUTTON3, INPUT_PULLUP);

  attachButtons();
}

void blinkLed(void) {
  // switch Led state
  digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
}

void wakeScreen() {
  sleep_disable();
  display.clearDisplay();
  detachInterrupt(digitalPinToPCINT(BUTTON3));
  if (sleepState == 1) {
    sleepState = 2;
  }
}


void incrState() {
  sleep_disable();
  detachInterrupt(digitalPinToPCINT(BUTTON2));
  // if (sleepState == 1) {
  sleepState = 2;
  state = (state + 1) % 5;
  delay(1000);
  display.clearDisplay();
}

int getWakeTime() {
  if (sleepState == 2) {
    turnOnMPU6050();
    setWakeTime();
    presTime = wakeRefTime;
    batteryLevel = getBatteryPercent();
    sleepState = 0;
    display.ssd1306_command(SSD1306_DISPLAYON);
    return (0);
  } else {

    TimeSpan rtcusb = rtc.now() - wakeRefTime;
    return (rtcusb.totalseconds());
  }
}

void setPresTime() {
  presTime = rtc.now();
}


void setWakeTime() {
  wakeRefTime = rtc.now();
}


void showTime(int colon) {
  display.setCursor(0, 0);
  display.print(presTime.hour());
  if (colon > 0) {
    display.print(F(":"));
  }
  int m = presTime.minute();
  if (m < 10) {
    display.print(0);
  }
  display.print(m);
}

void showDate() {
  display.setCursor(0, 10);
  display.print(presTime.year());
  display.print(F("/"));
  display.print(presTime.month());
  display.print(F("/"));
  display.print(presTime.day());
}

// watchdog code from http://tripsintech.com/arduino-sleep/
// Watchdog Prescalars
#define NUMPRESCALARS 10
static const uint8_t  PROGMEM prescalars[NUMPRESCALARS] = {9, 8, 7, 6, 5, 4, 3, 2, 1, 0};
static const long  PROGMEM prescalartimes[NUMPRESCALARS] = {8000, 4000, 2000, 1000, 500, 250, 128, 64, 32, 16};

ISR(WDT_vect) {
  // Do nothing.
}

/// Sleeps the arduino for a number of milliseconds
void sleepMillis(long mi) {

  // OPTIONAL delay to wait for all things to finish, e.g. serial prints - else you may get garbled serial prints from sleeping before the sending has finished.
  //delay(50);

  uint8_t prescalar = 0;

  // Sleep for the longest possible watchdog timeout that's less than millis and keep going until there are no millis left.
  while (mi > 16) {
    for (int i = 0; i < NUMPRESCALARS; ++i) {
      long pt = pgm_read_word_near(prescalartimes + i);
      if (mi > pt) {
        prescalar = pgm_read_word_near(prescalars + i);
        //prescalar = prescalars[i];
        mi -= pt;
        break;
      }
    }
    setupWatchdog(prescalar);
    doSleep();
  }
}

/// Sets up the watchdog to timeout after a certain time period.
/// There are many comments / notes in this function which have been copied directly from the data sheet for
/// user convenience.
void setupWatchdog(uint8_t prescalar) {

  // Prescalars can be: 0=16ms, 1=32ms, 2=64ms, 3=125ms, 4=250ms, 5=500ms, 6=1sec, 7=2sec, 8=4sec, 9=8sec
  if (prescalar > 9) {
    prescalar = 9;
  }

  // _BV is a macro that can be thought of as a funtion that takes in a number and outputs a byte with that bit set.

  // WDTCSR - Watchdog Timer Control Register
  // Note that WDP[0-3] is not in order (WDP[0-2] is but WDP3 is actually bit 5 not 3!) so we have to preprocess the prescaler passed in above.
  // bits 7 = WDIF, 6 = WDIE, 5 = WDP3, 4 = WDCE, 3 = WDE, 2 = WDP2, 1 = WDP1, 0 = WDP0

  // WDP3 WDP2 WDP1 WDP0 Typical Time-out at VCC = 5.0V
  // 0 0 0 0 16ms
  // 0 0 0 1 32ms
  // 0 0 1 0 64ms
  // 0 0 1 1 0.125 s
  // 0 1 0 0 0.25 s
  // 0 1 0 1 0.5 s
  // 0 1 1 0 1.0 s
  // 0 1 1 1 2.0 s
  // 1 0 0 0 4.0 s
  // 1 0 0 1 8.0 s

  // Take the first 3 bits (WDP[0-2])
  uint8_t wdtPrescalarBits = prescalar & 7;

  // Now we need to set WDP3, to do this we don't set bit 3 but bit 5, so if our presclar had bit 8 set i.e. it
  // was 8 or 9 being passed in then we must set WDP3 accordingly, else we could have just used prescar as it was passed in.
  if ( prescalar & 8 ) {
    wdtPrescalarBits |= _BV(WDP3);
  }

  // MCUSR – MCU Status Register
  // The MCU Status Register provides information on which reset source caused an MCU reset.
  // MCUSR Bit 3 – WDRF: Watchdog System Reset Flag
  // This bit is set if a Watchdog System Reset occurs. The bit is reset by a Power-on Reset, or by writing a logic zero to the flag.
  MCUSR &= ~_BV(WDRF);

  // WDTCSR Bit 4 – WDCE: Watchdog Change Enable
  // This bit is used in timed sequences for changing WDE and prescaler bits. To clear the WDE bit, and/or change the prescaler bits, WDCE must be set.
  // Once written to one, hardware will clear WDCE after four clock cycles.

  // WDTCSR Bit 3 – WDE: Watchdog System Reset Enable
  // WDE is overridden by WDRF in MCUSR. This means that WDE is always set when WDRF is set. To clear
  // WDE, WDRF must be cleared first. This feature ensures multiple resets during conditions causing failure, and a
  // safe start-up after the failure

  // Allow changes
  WDTCSR = _BV(WDCE) | _BV(WDE);

  // WDTCSR Bit 6 – WDIE: Watchdog Interrupt Enable
  // When this bit is written to one and the I-bit in the Status Register is set, the Watchdog Interrupt is enabled. If WDE is cleared in combination with this setting, the Watchdog Timer is in Interrupt Mode, and the corresponding interrupt is executed if time-out in the Watchdog Timer occurs. If WDE is set, the Watchdog Timer is in Interrupt and System Reset Mode. The first time-out in the Watchdog Timer will set WDIF. Executing the corresponding interrupt vector will clear WDIE and WDIF automatically by hardware (the Watchdog goes to System Reset Mode). This is useful for keeping the Watchdog Timer security while using the interrupt. To stay in Interrupt and System Reset Mode, WDIE must be set after each interrupt. This should however not be done within the interrupt service routine itself, as this might compromise the safety-function of the Watchdog System Reset mode. If the interrupt is not executed before the next time-out, a System Reset will be applied.
  // Note: 1. WDTON Fuse set to "0" means programmed and "1" means unprogrammed.

  // Watchdog Timer Configuration
  //WDTON WDE WDIE Mode Action on Time-out
  //1 0 0 Stopped None
  //1 0 1 Interrupt Mode Interrupt
  //1 1 0 System Reset Mode Reset
  //1 1 1 Interrupt and System Reset Mode Interrupt, then go to System Reset Mode
  //0 x x System Reset Mode Reset

  // Perform the change.
  WDTCSR = _BV(WDCE) | wdtPrescalarBits | _BV(WDIE);
}

/// Powers down system.
void doSleep() {

  // Set the sleep mode.
  /* Now is the time to set the sleep mode. In the Atmega8 datasheet
     http://www.atmel.com/dyn/resources/prod_documents/doc2486.pdf on page 35
     there is a list of sleep modes which explains which clocks and
     wake up sources are available in which sleep mode.

     In the avr/sleep.h file, the call names of these sleep modes are to be found:

     The 5 different modes are:
         SLEEP_MODE_IDLE         -the least power savings
         SLEEP_MODE_ADC
         SLEEP_MODE_PWR_SAVE
         SLEEP_MODE_STANDBY
         SLEEP_MODE_PWR_DOWN     -the most power savings

     For now, we want as much power savings as possible, so we
     choose the according
     sleep mode: SLEEP_MODE_PWR_DOWN

  */
  set_sleep_mode(SLEEP_MODE_PWR_DOWN);   // sleep mode is set here

  sleep_enable();          // enables the sleep bit in the mcucr register
  // so sleep is possible. just a safety pin

  /* Now it is time to enable an interrupt. We do it here so an
     accidentally pushed interrupt button doesn't interrupt
     our running program. if you want to be able to run
     interrupt code besides the sleep function, place it in
     setup() for example.

     In the function call attachInterrupt(A, B, C)
     A   can be either 0 or 1 for interrupts on pin 2 or 3.

     B   Name of a function you want to execute at interrupt for A.

     C   Trigger mode of the interrupt pin. can be:
                 LOW        a low level triggers
                 CHANGE     a change in level triggers
                 RISING     a rising edge of a level triggers
                 FALLING    a falling edge of a level triggers

     In all but the IDLE sleep modes only LOW can be used.
  */
  attachButtons();

  sleep_mode();            // here the device is actually put to sleep!!
  // THE PROGRAM CONTINUES FROM HERE AFTER WAKING UP

  sleep_disable();         // first thing after waking from sleep:
  // disable sleep...

}

bool IsUSBConnected() {
  return (UDADDR & _BV(ADDEN));
}


void interruptSleep()         // here we put the arduino to sleep
{
  display.ssd1306_command(SSD1306_DISPLAYOFF);
  turnOffMPU6050();
  sleepState = 1;

  WDTCSR  = (0 << WDP3) | (1 << WDP2) | (1 << WDP1) | (1 << WDP0); // sleep as long as possible

  //power_all_disable ();  // turn off various modules
  power_adc_disable();   // disable ADC

  doSleep();  // go to sleep

  // turn everything back on
  power_adc_enable(); // ADC converter
  /* power_spi_enable(); // SPI
    power_usart0_enable(); // Serial (USART)
    power_timer0_enable(); // Timer 0
    power_timer1_enable(); // Timer 1
    power_timer2_enable(); // Timer 2
    power_twi_enable(); // TWI (I2C)*/

}


void sleepControl() {
  if (getWakeTime() > 10) {
    interruptSleep();
  }
}

