
float bulletangle[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
char bullettime[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

static const int PROGMEM bigasteriodx[] = { -1, -2, -1, 0, 0, 1, 1, 2, 2, 1, -1};
static const int PROGMEM bigasteriody[] = { -2, -1,  0, 2, 2, 1, 0, -1, -2, -2, -2};
char asteriodmult[] = { -1, -1, -1};
char asteriodvx[] = {1, -1, -1};
char asteriodvy[] = {1, 1, -1};
char asteriodx[] = { -1, -1, -1};
char asteriody[] = { -1, -1, -1};

#define NUMSIDESASTEROIDS (sizeof(bigasteriodx) / sizeof(int))
#define NUMBULLETS 10

byte levelAsteroids = 0;
int pointsAsteroids = 0;
byte numShips = 3;
char bulletindex = 0;

float lastyy;
float angle = -3;

void resetAsteriods() {
  for (int i = 0; i < 2; i++) {
    asteriodmult[i] = -1;
    asteriodvx[i] = -1;
    asteriodvy[i] = -1;
    asteriodx[i] = -1;
    asteriody[i] = -1;
  }
}

float dist2(int x, int y, int x1, int y1) {
  float dx = x - x1;
  float dy = y - y1;
  return (dx * dx + dy * dy);
}

float computeChaseAngle() {
  int dista = 32000;
  if (asteriodmult[0] > 1) {
    dista = dist2(MIDSCREENX, MIDSCREENY, asteriodx[0], asteriody[0]);
  }
  int distb = 32000;
  if (asteriodmult[1] > 1) {
    distb = dist2(MIDSCREENX, MIDSCREENY, asteriodx[1], asteriody[1]);
  }
  int tar = 0;
  if (distb < dista) {
    tar = 1;
  }

  float yy = asteriody[tar] + 12 * asteriodvy[tar] - MIDSCREENY;
  float xx = asteriodx[tar] + 12 * asteriodvx[tar] - MIDSCREENX;
  float tarangle = atan2(yy, xx);

  if (yy >= 0 && lastyy < 0) {
    angle = PI * 2 + angle;
  }
  float diffangle = (tarangle - angle);
  angle = angle +  0.2 * diffangle;
  lastyy = yy;
}

void createBullet() {
  if (bulletindex == NUMBULLETS) {
    bulletindex = 0;
  }

  bulletangle[bulletindex] = angle;
  bullettime[bulletindex] = 1;
  bulletindex = bulletindex + 1;
}



void drawAsteroidsGame(int roll) {

  drawAllAsteroids();
  drawShip(angle, 8, MIDSCREENX, MIDSCREENY);
  drawBullets(angle);
    drawBackupShip();
    
  if (count % 2 == 0) {
    createBullet();
  }

  if (roll == -1000) {
    computeChaseAngle();
      levelAsteroids=0;
  } else {
    display.setCursor(50, 0);
    display.print(pointsAsteroids);

    if (roll > -30) {
      angle = angle + PI / 16;
    }
    if (roll < -35) {
      angle = angle - PI / 16;
    }
  }
  if (angle > PI * 2) {
    angle = 0;
  }
  sleepMillis(125 - levelAsteroids * 20);
}

void drawShip(float dir, int shipsize, int x, int y) {
  float tip = 1.2;
  float back = 1;
  int tx = x + tip * shipsize * cos(dir);
  int ty = y + tip * shipsize * sin(dir);
  int bx1 = x + -back * shipsize * cos(dir + PI / 3);
  int by1 = y + -back * shipsize * sin(dir + PI / 3);
  int bx2 = x + -back * shipsize * cos(dir - PI / 3);
  int by2 = y + -back * shipsize * sin(dir - PI / 3);

  display.drawLine(tx, ty, bx1, by1 , 1);
  display.drawLine(tx, ty, bx2, by2 , 1);
  display.drawLine(bx1, by1, bx2, by2, 1);
}


void drawExplodedShip(float dir, int shipsize, int x, int y) {
  float tip = 1.2;
  float back = 1;
  byte tx = x + tip * shipsize * cos(dir);
  byte  ty = y + tip * shipsize * sin(dir);
  byte  bx1 = x + -back * shipsize * cos(dir + PI / 3);
  byte  by1 = y + -back * shipsize * sin(dir + PI / 3);
  byte  bx2 = x + -back * shipsize * cos(dir - PI / 3);
  byte  by2 = y + -back * shipsize * sin(dir - PI / 3);
  byte  txB = x + tip * shipsize * cos(dir + PI4);
  byte  tyB = y + tip * shipsize * sin(dir + PI4);
  byte  bx1B = x + -back * shipsize * cos(dir + PI4 + PI / 3);
  byte  by1B = y + -back * shipsize * sin(dir + PI4 + PI / 3);
  byte  bx2B = x + -back * shipsize * cos(dir + PI4 - PI / 3);
  byte  by2B = y + -back * shipsize * sin(dir + PI4 - PI / 3);

  display.drawLine(tx, ty, bx1B, by1B , 1);
  // display.drawLine(bx1h, by1h+5, bx1, by1 , 1);
  display.drawLine(txB, tyB, bx2, by2 , 1);
  display.drawLine(bx1, by1, bx2B, by2B, 1);
}


void drawBackupShip() {
  if (numShips > 0) {
    drawShip(PI / 2 + PI, 4, 4, 13);
  }
  if (numShips > 1) {
    drawShip(PI / 2 + PI, 4, 12, 13);
  }
  if (numShips > 2) {
    drawShip(PI / 2 + PI, 4, 20, 13);
  }
}


void drawAllAsteroids() {
  for (int i = 0; i < 2; i++) {
    if (asteriodx[i] == -1) {
      createNewAsteriods(i);
    } else {
      //   if (asteriodtime[i] <= 0) {
      if (asteriodmult[i]  < -1) {
        drawExplosion(i);
      } else {
        if (asteriodmult[i] > 1 && asteriodmult[i] <= 3) {
          asteriodx[i] = asteriodx[i] + asteriodvx[i];
          asteriody[i] = asteriody[i] + asteriodvy[i];
          drawAsteroid(asteriodx[i], asteriody[i], asteriodmult[i]);

        }
      }
      //     if (asteriodmult[i] > 20) {
      if (asteriodx[i] < -5 || asteriodx[i] > SCREENX + 5) {
        asteriodmult[i] = -1;
        asteriodx[i] = -1;

      }
      if (asteriody[i] < -5 || asteriody[i] > SCREENY + 5) {
        asteriodmult[i] = -1;
        asteriodx[i] = -1;
      }
      //    }
      float dist = dist2(MIDSCREENX, MIDSCREENY, asteriodx[i], asteriody[i]);
      if (dist < 40) {
        if (numShips == 0) {
          display.setCursor(30, 10);
          display.print(F("Game Over"));
          display.display();
          sleepMillis(1000);
          pointsAsteroids = 0;
          numShips = 3;
        } else {
          drawExplodedShip(angle, 12, MIDSCREENX, MIDSCREENY);
          display.display();
          numShips = numShips - 1;
        }
        sleepMillis(1000);
        resetAsteriods();
      }

    }
  }
}

void createNewAsteriods(int i) {
  if (count > 15) {
    float astdir = random(0, TWOPI);
    asteriodmult[i] = 3;
    asteriodx[i] = MIDSCREENX + cos(astdir) * SCREENX * 0.45;
    asteriody[i] = MIDSCREENY + sin(astdir) * SCREENY * 0.45;
    // asteriodtime[i] = 0;
    asteriodvx[i] = (MIDSCREENX - asteriodx[i] + random(0, 20) - 5) * 0.05;
    asteriodvy[i] = (MIDSCREENY - asteriody[i] + random(0, 20) - 10) * 0.05;
    count = 0;
  }
}


void drawExplosion(int i) {
  int mul = 1;
  display.drawPixel(asteriodx[i], asteriody[i] - asteriodmult[i]*mul, 1);
  display.drawPixel(asteriodx[i], asteriody[i] + asteriodmult[i]*mul, 1);
  display.drawPixel(asteriodx[i] + asteriodmult[i]*mul, asteriody[i] - asteriodmult[i]*mul, 1);
  display.drawPixel(asteriodx[i] + asteriodmult[i]*mul, asteriody[i] + asteriodmult[i]*mul, 1);
  display.drawPixel(asteriodx[i] - asteriodmult[i]*mul, asteriody[i] - asteriodmult[i]*mul, 1);
  display.drawPixel(asteriodx[i] - asteriodmult[i]*mul, asteriody[i] + asteriodmult[i]*mul, 1);
  display.drawPixel(asteriodx[i] - asteriodmult[i]*mul, asteriody[i], 1);
  display.drawPixel(asteriodx[i] + asteriodmult[i]*mul, asteriody[i], 1);
  asteriodmult[i] = asteriodmult[i] - 1; // explosion gets bigger
  if (asteriodmult[i] < -6) {
    createNewAsteriods(i);
  }
}

void drawAsteroid(int cx, int cy, int mult) {
  //    int mult = 3;
  if (cx > 0 && cx < SCREENX && cy > 0 && cy < SCREENY) {
    int bdx = 0;
    int bdy = 0;
    for (int i = 1; i < NUMSIDESASTEROIDS; i++) {
      int lastbdx = pgm_read_word_near(bigasteriodx + i - 1);
      int lastbdy = pgm_read_word_near(bigasteriody + i - 1);
      bdx = pgm_read_word_near(bigasteriodx + i);
      bdy = pgm_read_word_near(bigasteriody + i);
      int x1 = lastbdx * mult;
      int y1 = lastbdy * mult;
      int x2 = bdx * mult;
      int y2 = bdy * mult;
      display.drawLine(cx + x1, cy + y1, cx + x2, cy + y2, 1);
    }
  }
}

void checkBulletAsteroidCollision(int bx, int by) {
  for (int j = 0; j < 2; j++) {

    float dist = dist2(bx, by, asteriodx[j], asteriody[j]);
    if (dist < 50) {
      if (dist < 20 && asteriodmult[j] > 0) {
        asteriodmult[j] = -2; // explosion
        pointsAsteroids = pointsAsteroids + 10;
      }
      if (asteriodmult[j] > 2) {
        asteriodmult[j] = 2; //smaller asteroid
        pointsAsteroids = pointsAsteroids + 10;
      }
    }
  }
}

void drawBullets(float dir) {
  int cx = 128 / 2;
  int cy = 64 / 2;
  int bulletspeed = 2;

  for (int i = 0; i < NUMBULLETS; i++) {
    if (bullettime[i] > 0) {
      float ba = bulletangle[i];

      bullettime[i] = bullettime[i] + 1;
      float bx = cx + cos(ba) * (8 + bullettime[i] * bulletspeed);
      float by = cy + sin(ba) * (8 + bullettime[i] * bulletspeed);

      display.drawPixel(bx, by, 1);
      if (bx > 2 && bx < SCREENX - 2  && by > 2 && by < SCREENY - 2) {
        checkBulletAsteroidCollision(bx, by);
      }
    }
  }
}
