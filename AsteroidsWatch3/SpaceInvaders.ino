
byte invadersMatrix[] = {
  1, 1, 1, 1,
  1, 1, 1, 1
};

static const unsigned char PROGMEM invaders[] =
{

  B00000000, B00000000,
  B00000000, B00000000,
  B00001100, B00110000,
  B00000110, B01100000,
  B00000011, B11000000,
  B00111111, B11111100,
  B01111001, B10011110,
  B11011001, B10011011,
  B11011111, B11111011,
  B11011111, B11111011,
  B00011000, B00011000,
  B00001110, B01110000,
  B00001110, B01110000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,

  B00000000, B00000000,
  B00000000, B00000000,
  B00001100, B00110000,
  B11000110, B01100011,
  B11000011, B11000011,
  B11111111, B11111111,
  B01111001, B10011110,
  B00011001, B10011000,
  B00011111, B11111000,
  B00011111, B11111000,
  B00011000, B00011000,
  B00110000, B00001100,
  B01100000, B00000110,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,

  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000000, B00000000,
  B00000001, B10000000,
  B00000011, B11000000,
  B00000011, B11000000,
  B00111111, B11111100,
  B01111111, B11111110,
  B01111111, B11111110,
  B00000000, B00000000,
  B00000000, B00000000
};

char hpos = 20;
char hdir = 1;
char vpos = 0;
char invtype = 0;
char spacing = 22;
char basepos = 20;
char basedir = 5;

int points = 0;

char missiletime[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
char missilex[] = { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1};
char lastmissile = 0;
char level = 0;

#define NUMINVADERS (sizeof(invadersMatrix) / sizeof(byte))
#define NUMMISSILES (sizeof(missiletime) / sizeof(char))

void resetInvaders() {
  hpos = 20;
  hdir = 1;
  vpos = 0;
  invtype = 0;
  spacing = 22;
  basepos = 20;
  basedir = 5;

  for (int i = 0; i < NUMINVADERS; i++) {
    invadersMatrix[i] = 1;
  }

  for (int i = 0; i < NUMMISSILES; i++) {
    missiletime[i] = -1;
    missilex[i] = -1;
  }
}

void drawInvaders() {
  int cinvaders = 0;
  for (int i = 0; i < NUMINVADERS; i++) {
    int row = i / 4;
    if (invadersMatrix[i] > 0) {
      invtype = 16 * 2 * ((row + count) % 2) + 2;

      display.drawBitmap(hpos + spacing * (i % 4), 18 * row + vpos,  &invaders[invtype], 16, 16, 1);

      if ( 18 * row + vpos > 45) {
        display.setCursor(30, 0);
        display.print(F("Game Over"));
        display.display();
        sleepMillis(2000);
        level = 0;
        resetInvaders();
      }
      cinvaders = cinvaders + 1;
    }
  }
  if (cinvaders == 0) {
    level = level + 1;
    if (level > 6) {
      level = 6;
    }
    resetInvaders();
  }
}

void drawMissileBase(int bpos) {
  //
  display.drawBitmap(bpos, 48,  &invaders[16 * 2 * 2], 16, 16, 1);
  // lastmissile=lastmissile+1%nummissiles;
}

void checkMissileCollision(int bx, int by, int mi) {
  for (int i = 0; i < NUMINVADERS; i++) {
    int row = i / 4;
    if (invadersMatrix[i] > 0) {
      float dist = dist2(bx, by, hpos + spacing * (i % 4) + 8, 18 * row + vpos);
      if (dist < 30) {
        invadersMatrix[i] = 0;
        missiletime[mi] = -1;
        missilex[mi] = -1;
        points = points + 10;
      }
    }
  }
}

void drawMissiles() {
  int bulletspeed = 2;

  for (int i = 0; i < NUMMISSILES; i++) {
    if (missilex[i] > 0) {
      missiletime[i] = missiletime[i] + 1;
      float bx = missilex[i] + 8;
      float by = 60 - (8 + missiletime[i] * bulletspeed);

      display.drawPixel(bx, by, 1);
      display.drawPixel(bx + 1, by, 1);
      display.drawPixel(bx + 1, by + 1, 1);
      display.drawPixel(bx, by + 1, 1);
      if (missiletime[i] * bulletspeed > 55) {
        missiletime[i] = -1;
        missilex[i] = -1;
      }
      checkMissileCollision(bx, by, i);

    }
  }
}


void drawInvadersGame(int roll) {
  hpos = hpos + hdir;
  if (hpos < 20) {
    hdir = 1;
    vpos = vpos + 5;
  }
  if (hpos > 30) {
    hdir = -1;
    vpos = vpos + 5;
  }

  if (roll == -1000) {
    basepos = basepos + basedir;
    if (basepos > 90) {
      basedir = -5;
    }
    if (basepos < 20) {
      basedir = 5;
    }
  } else {
    if (roll > 10) {
      basepos = basepos + 5;
    }
    if (roll < -10) {
      basepos = basepos - 5;
    }
    if (basepos > 90) {
      basepos = 90;
    }
    if (basepos < 20) {
      basepos = 20;
    }
  }

  if (count % 6 == 0) {
    missilex[lastmissile] = basepos;
    lastmissile = lastmissile + 1;
    lastmissile = lastmissile % NUMMISSILES;
  }
  drawMissileBase(basepos);
  drawMissiles();
  display.setCursor(110, 0);
  if (points < 10) {
    display.print(F("00"));
  }
  if (points < 100) {
    display.print(F("0"));
  }
  display.print(points);

  drawInvaders();

  sleepMillis(250 - level * 40);
}
