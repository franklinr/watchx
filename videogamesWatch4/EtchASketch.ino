
int cursorx = SCREENX / 2, cursory = SCREENY / 2;

void runEtchASketch() {
  Vector normAccel = mpu.readNormalizeAccel();
  int pitch = -(atan2(normAccel.XAxis, sqrt(normAccel.YAxis * normAccel.YAxis + normAccel.ZAxis * normAccel.ZAxis)) * 180.0) / M_PI;
  int roll = (atan2(normAccel.YAxis, normAccel.ZAxis) * 180.0) / M_PI;

  if (pitch > 10 - 30) {
    cursory = cursory - 1;
  }
  if (pitch < -10 - 30) {
    cursory = cursory + 1;
  }
  if (roll > 10) {
    cursorx = cursorx + 1;
  }
  if (roll < -10) {
    cursorx = cursorx - 1;
  }
  if (cursorx < 0) {
    cursorx = 1;
  }
  if (cursorx > SCREENX - 2) {
    cursorx = SCREENX - 2;
  }
  if (cursory < 0) {
    cursory = 1;
  }
  if (cursory > SCREENY - 2) {
    cursory = SCREENY - 2;
  }
  display.drawPixel(cursorx, cursory, 1);

  display.display();
  sleepMillis(150);
}
