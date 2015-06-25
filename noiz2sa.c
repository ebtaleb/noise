/*
 * $Id: noiz2sa.c,v 1.8 2003/02/12 13:55:13 kenta Exp $
 *
 * Copyright 2002 Kenta Cho. All rights reserved.
 */

/**
 * Noiz2sa main routine.
 *
 * @version $Revision: 1.8 $
 */
#include "SDL.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "noiz2sa.h"
#include "screen.h"
#include "vector.h"
#include "brgmng_mtd.h"
#include "degutil.h"
#include "foe_mtd.h"

static int noSound = 0;

// Initialize and load preference.
static void initFirst() {
  srand(SDL_GetTicks());
  initBarragemanager();
}

// Quit and save preference.
void quitLast() {
  closeBarragemanager();
  closeSDL();
  SDL_Quit();
  exit(1);
}

int status;

void initGame() {
  status = IN_GAME;

  initBarrages();
  initFoes();
}

static void move() {
    addBullets();
    moveFoes();
}

static void draw() {
    drawBullets();
}

static int accframe = 0;

static void usage(char *argv0) {
  fprintf(stderr, "Usage: %s [-nosound] [-window] [-reverse] [-brightness n] [-nowait] [-accframe]\n", argv0);
}

static void parseArgs(int argc, char *argv[]) {
  int i;
  for ( i=1 ; i<argc ; i++ ) {
    if ( strcmp(argv[i], "-nosound") == 0 ) {
      noSound = 1;
    } else if ( strcmp(argv[i], "-window") == 0 ) {
      windowMode = 1;
    } else if ( strcmp(argv[i], "-reverse") == 0 ) {
      buttonReversed = 1;
    } else if ( (strcmp(argv[i], "-brightness") == 0) && argv[i+1] ) {
      i++;
      brightness = (int)atoi(argv[i]);
      if ( brightness < 0 || brightness > 256 ) {
        brightness = DEFAULT_BRIGHTNESS;
      }
    } else if ( strcmp(argv[i], "-accframe") == 0 ) {
      accframe = 1;
    } else {
      usage(argv[0]);
      exit(1);
    }
  }
}

int interval = INTERVAL_BASE;
int tick = 0;
static int pPrsd = 1;

int main(int argc, char *argv[]) {
  int done = 0;
  long prvTickCount = 0;
  int i;
  int btn;
  SDL_Event event;
  long nowTick;
  int frame;

  parseArgs(argc, argv);

  initDegutil();
  initSDL(windowMode);
  initFirst();
  initGame();

  while ( !done ) {
    SDL_PollEvent(&event);
    keys = SDL_GetKeyState(NULL);
    if ( keys[SDLK_ESCAPE] == SDL_PRESSED || event.type == SDL_QUIT ) done = 1;
    if ( keys[SDLK_p] == SDL_PRESSED ) {
      if ( !pPrsd ) {
	if ( status == IN_GAME ) {
	  status = PAUSE;
	} else if ( status == PAUSE ) {
	  status = IN_GAME;
	}
      }
      pPrsd = 1;
    } else {
      pPrsd = 0;
    }

    nowTick = SDL_GetTicks();
    frame = (int)(nowTick-prvTickCount) / interval;
    if ( frame <= 0 ) {
      frame = 1;
      SDL_Delay(prvTickCount+interval-nowTick);
      if ( accframe ) {
	prvTickCount = SDL_GetTicks();
      } else {
	prvTickCount += interval;
      }
    } else if ( frame > 5 ) {
      frame = 5;
      prvTickCount = nowTick;
    } else {
      prvTickCount += frame*interval;
    }
    for ( i=0 ; i<frame ; i++ ) {
      move();
      tick++;
    }
    smokeScreen();
    draw();
    flipScreen();
  }
  quitLast();
}
