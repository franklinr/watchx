# sensorHistory

This program lets you monitor the sensors of watchx to better understand how they work.  

![Alt Text](sensorHistory.gif)

It shows a graph for each sensor, which automatically adjusts for the range of the data.  You can press the right bottom button to force recalibration of the range.

The first page is pitch from the accelerometer MPU6050.  Pressing the top left button will get you to the accelerometer roll reading.  Press it again to get to the MAG3110 magnetometer heading reading (0 is north, it requires some calibration on startup).  Then next pages have temperature and pressure from the Barometric Pressure Sensor (BMP280).  The final page is voltage.

Pressing the top right button will change the timing of recordings.  It can record once per hour (3600000 ms), once per minute (60000 ms), once per second (1000 ms), or as fast as the processor can go (1 ms).

In the code is this line
`#define USEEEPROM`
It is normally commented out and so the program uses variables in ram to store the sensor readings.  But sometimes you want to store data permanently, even if the battery dies.  Then uncomment this line and the program will save the readings in the EEPROM, which is like a harddrive.  I used this to record the voltage of the battery when it died.  When you unplug the USB, the led will turn on and the system will record the time, so you can see how long the system has been disconnected from USB.  When you plug the USB back in after it dies, you should get the graph for the system right up until it died.  I got a voltage of 3.22 and that is slightly lower than the other values that I have seen (maybe because serial and the screen were off when it died).  There is a limit to the number of times you can save things in your EEPROM, so you don't want to leave the EEPROM version on with a high record rate.

You will need to download the PinChangeInterrupt Library and put it in your arduino libraries folder.
https://github.com/NicoHood/PinChangeInterrupt
