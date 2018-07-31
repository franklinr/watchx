# Asteroids/Space Invaders Watchfaces

TWo watchfaces for watchX which let you play asteriods and space invaders.  

![Alt Text](asteroids.gif)

Time is shown as the score (in the video, it is 13:58).  Battery life is shown as a percent.

You can now play asteroids by pressing the top right button and rolling your wrist towards you or away from you.

When the watch is disconnected from USB, it will sleep after a few seconds and is woken with an interrupt.  In this mode, you should have a long battery life.  But in this mode, you cannot program the watch.  To program the watch, plug in the USB cord and press the button on the opposite side from the cord.  That will activating programming mode and you will see three ships on the top right (two ships for battery mode).  

If you press the same button again, it will switch to space invaders.  Again, time is the score.

If you press the top right button, then you can move the ship back and forth by changing the angle of your wrist up or down.

![Alt Text](invaders.gif)

If you press the same button again, it will switch to this [Matrix-like raining code](https://www.youtube.com/watch?v=SneR61OG4ZI&t=38s) page.  The code is actually Japanese for year, month, date, hour, min, and battery life (from right to left).
![Alt Text](matrixj.gif)
Since there is no space in the watch, the code for Matrix-like rain has been turned off, but the code is still available and can be turned on by uncommenting code in the loop.  

Pressing the button again and you will get a screen with date, time, battery charge (%), time since the usb was removed (minutes), change in battery level since usb was removed, and estimated battery life in minutes.

Pressing the button again returns you to asteroids.

To run, you will need to install the EnableInterrupt library https://github.com/GreyGnome/EnableInterrupt and the arduino-mpu6050 library.  Both are available in the Library Manager in Arduino IDE.

[![IMAGE ALT TEXT HERE](https://www.youtube.com/watch?v=PPMMDkDDmgg/0.jpg)](https://www.youtube.com/watch?v=PPMMDkDDmgg)
