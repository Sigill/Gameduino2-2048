#include <TrueRandom.h>

#include <EEPROM.h>
#include <SPI.h>
#include <GD2.h>

class Swipper {
public:
  int32_t ox, oy, dx, dy;
  unsigned long lastTouchDate;
  bool swipping, swipped;

  Swipper() :
    swipping(false),
    swipped(false),
    lastTouchDate(millis())
  {};

  void update() {
    if(GD.inputs.x != -32768) {
      lastTouchDate = millis();
      if(!swipping) {
        swipping = true;
        ox = GD.inputs.x;
        oy = GD.inputs.y;
      }

      dx = GD.inputs.x - ox;
      dy = GD.inputs.y - oy;
    } else {
      if(millis() - lastTouchDate > 50) {
        if(swipping) {
          swipped = true;
          swipping = false;
        }
      }
    }
  }
};

Swipper swipper;
char _game[16] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
char *game[4] = {&(_game[0]), &(_game[4]), &(_game[8]), &(_game[12])};
uint32_t score = 0;

uint32_t COLOR[] = {0xeee4da, 0xece0c8, 0xeb8c53, 0xec8c54, 0xf57c5f, 0xe95937, 0xf2d86a, 0xedcc61, 0xecc850, 0xedc53f, 0xecc22e, 0x5fda92};
byte FONT_SIZE[] = {31, 31, 31, 31, 31, 31, 30, 30, 30, 29, 29, 29};

void randStep() {
  char *sp = _game, *ep = _game + 16;
  char c = 0;
  while(sp < ep) {
    if(0 == *sp)
      ++c;
    ++sp;
  }

  char p = (char)random(0, c);

  sp = _game;
  c = 0;
  while(sp < ep) {
    if(0 == *sp) {
      if(c == p) {
        *sp = random(0, 8) ? 1 : 2;
        break;
      }
      ++c;
    }
    ++sp;
  }
}

bool swipeLeft(uint32_t *points) {
  bool somethingMoved = false;
  for(int y = 0; y < 4; ++y) {
    for(int x = 0; x < 3; ++x) {
      if(game[x][y] > 0) {
        for(int i = x + 1; i < 4; ++i) {
          if(game[i][y] > 0) {
            if(game[i][y] == game[x][y]) {
              ++game[x][y];
              game[i][y] = 0;
              *points += 1 << game[x][y];
              somethingMoved = true;
            }
            break;
          }
        }
      }
    }

    for(int x = 0; x < 3; ++x) {
      if(game[x][y] == 0) {
        for(int i = x + 1; i < 4; ++i) {
          if(game[i][y] != 0) {
            game[x][y] = game[i][y];
            game[i][y] = 0;
            somethingMoved = true;
            break;
          }
        }
      }
    }
  }

  return somethingMoved;
}

bool swipeRight(uint32_t *points) {
  bool somethingMoved = false;
  for(int y = 0; y < 4; ++y) {
    for(int x = 3; x > 0; --x) {
      if(game[x][y] > 0) {
        for(int i = x - 1; i >= 0; --i) {
          if(game[i][y] > 0) {
            if(game[i][y] == game[x][y]) {
              ++game[x][y];
              game[i][y] = 0;
              *points += 1 << game[x][y];
              somethingMoved = true;
            }
            break;
          }
        }
      }
    }

    for(int x = 3; x > 0; --x) {
      if(game[x][y] == 0) {
        for(int i = x - 1; i >= 0; --i) {
          if(game[i][y] != 0) {
            game[x][y] = game[i][y];
            game[i][y] = 0;
            somethingMoved = true;
            break;
          }
        }
      }
    }
  }

  return somethingMoved;
}

bool swipeDown(uint32_t *points) {
  bool somethingMoved = false;
  for(int x = 0; x < 4; ++x) {
    for(int y = 3; y > 0; --y) {
      if(game[x][y] > 0) {
        for(int i = y - 1; i >= 0; --i) {
          if(game[x][i] > 0) {
            if(game[x][i] == game[x][y]) {
              ++game[x][y];
              game[x][i] = 0;
              *points += 1 << game[x][y];
              somethingMoved = true;
            }
            break;
          }
        }
      }
    }

    for(int y = 3; y > 0; --y) {
      if(game[x][y] == 0) {
        for(int i = y - 1; i >= 0; --i) {
          if(game[x][i] != 0) {
            game[x][y] = game[x][i];
            game[x][i] = 0;
            somethingMoved = true;
            break;
          }
        }
      }
    }
  }

  return somethingMoved;
}

bool swipeUp(uint32_t *points) {
  bool somethingMoved = false;
  for(int x = 0; x < 4; ++x) {
    for(int y = 0; y < 3; ++y) {
      if(game[x][y] > 0) {
        for(int i = y + 1; i < 4; ++i) {
          if(game[x][i] > 0) {
            if(game[x][i] == game[x][y]) {
              ++game[x][y];
              game[x][i] = 0;
              *points += 1 << game[x][y];
              somethingMoved = true;
            }
            break;
          }
        }
      }
    }

    for(int y = 0; y < 3; ++y) {
      if(game[x][y] == 0) {
        for(int i = y + 1; i < 4; ++i) {
          if(game[x][i] != 0) {
            game[x][y] = game[x][i];
            game[x][i] = 0;
            somethingMoved = true;
            break;
          }
        }
      }
    }
  }

  return somethingMoved;
}

void setup()
{
  Serial.begin(9600);
  GD.begin();
  //GD.cmd_calibrate();
  randomSeed(TrueRandom.random());
  randStep();
  randStep();
}

void loop()
{
  GD.get_inputs();

  GD.ClearColorRGB(0xbbada0);
  GD.Clear();

  swipper.update();

  if(swipper.swipped) {
    GD.cmd_number(360, 100, 31, OPT_CENTER | OPT_SIGNED, swipper.dx);
    GD.cmd_number(360, 200, 31, OPT_CENTER | OPT_SIGNED, swipper.dy);

    bool somethingMoved = false;

    if(millis() - swipper.lastTouchDate > 500) {
      uint32_t points = 0;
      if(abs(swipper.dx) > 2 * abs(swipper.dy) && abs(swipper.dx) > 20) {
        if(swipper.dx < 0) {
          somethingMoved = swipeLeft(&points);
        } else {
          somethingMoved = swipeRight(&points);
        }
      } else if(2 * abs(swipper.dx) < abs(swipper.dy)  && abs(swipper.dy) > 20) {
        Serial.print("Vertical: ");
        if(swipper.dy < 0) {
          somethingMoved = swipeUp(&points);
        } else {
          somethingMoved = swipeDown(&points);
        }
      }
      if(somethingMoved)
        randStep();
      score += points;
      swipper.swipped = false;
    }
  }

  GD.Begin(RECTS);
  GD.LineWidth(2 * 16);
  for(int x = 0; x < 4; ++x) {
    for(int y = 0; y < 4; ++y) {
      GD.Begin(RECTS);
      GD.LineWidth(2 * 16);

      if(game[x][y] == 0) {
        GD.ColorRGB(0xffffff);
      } else {
        GD.ColorRGB(COLOR[game[x][y] - 1]);
      }

      GD.Vertex2ii(8 + 29 + x * (8 + 58) - 28, 8 + 29 + y * (8 + 58) - 28);
      GD.Vertex2ii(8 + 29 + x * (8 + 58) + 28, 8 + 29 + y * (8 + 58) + 28);

      if(game[x][y] > 0) {
        GD.ColorRGB(0xffffff);
        GD.cmd_number(8 + 29 + x * (8 + 58), 8 + 29 + y * (8 + 58), FONT_SIZE[game[x][y] - 1], OPT_CENTER, 1 << game[x][y]);
      }
    }
  }

  GD.cmd_text(310, 20, 29, OPT_CENTER, "Score");
  GD.cmd_number(310, 40, 29, OPT_CENTER, score);

  GD.swap();
}
