#include <SDL.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef DEBUG
#include <sys/resource.h>
#endif

#include "noise.h"
#include "screen.h"
#include "vector.h"
#include "brgmng_mtd.h"
#include "degutil.h"
#include "foe_mtd.h"

static int core = 0;

static void initFirst() {
    srand(SDL_GetTicks());
    initBarragemanager();
}

void quitLast() {
    closeBarragemanager();
    closeSDL();
    SDL_Quit();
    exit(1);
}

void initGame() {
    initBarrages();
    initFoes();
}

static void usage(char *argv0) {
    fprintf(stderr, "Usage: %s [-window] [-reverse] [-brightness n]\n", argv0);
}

static void parseArgs(int argc, char *argv[]) {
    int i;
    for ( i=1 ; i<argc ; i++ ) {
        if ( strcmp(argv[i], "-window") == 0 ) {
            windowMode = 1;
        } else if ( strcmp(argv[i], "-reverse") == 0 ) {
            buttonReversed = 1;
        } else if ( (strcmp(argv[i], "-brightness") == 0) && argv[i+1] ) {
            i++;
            brightness = (int)atoi(argv[i]);
            if ( brightness < 0 || brightness > 256 ) {
                brightness = DEFAULT_BRIGHTNESS;
            }
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

#ifdef DEBUG
    struct rlimit core_limits;
    core_limits.rlim_cur = core_limits.rlim_max = RLIM_INFINITY;
    setrlimit(RLIMIT_CORE, &core_limits);
#endif

    initDegutil();
    initSDL(windowMode);
    initFirst();
    initGame();

    while ( !done ) {
        SDL_PollEvent(&event);
        keys = SDL_GetKeyState(NULL);

        if (event.type == SDL_MOUSEMOTION) {
            /*printf("%d, %d\n", event.motion.x, event.motion.y);*/
        }

        if ( keys[SDLK_ESCAPE] == SDL_PRESSED || event.type == SDL_QUIT )
            done = 1;

        if ( keys[SDLK_p] == SDL_PRESSED ) {
            if ( !pPrsd ) {
            }
            pPrsd = 1;
        } else {
            pPrsd = 0;
        }

        clearScreen();
        nowTick = SDL_GetTicks();
        frame = (int)(nowTick-prvTickCount) / interval;
        if ( frame <= 0 ) {
            frame = 1;
            SDL_Delay(prvTickCount+interval-nowTick);
            prvTickCount += interval;
        } else if ( frame > 5 ) {
            frame = 5;
            prvTickCount = nowTick;
        } else {
            prvTickCount += frame*interval;
        }
        for ( i=0 ; i<frame ; i++ ) {

            addBullets();
            moveFoes();

            tick++;
        }
        smokeScreen();
        drawBullets();
        flipScreen();
    }
    quitLast();
}
