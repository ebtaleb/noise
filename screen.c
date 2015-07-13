#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "SDL.h"

#include "noise.h"
#include "screen.h"
#include "clrtbl.h"
#include "vector.h"
#include "degutil.h"

int windowMode = 0;
int brightness = DEFAULT_BRIGHTNESS;

static SDL_Surface *video, *layer;
static LayerBit **smokeBuf;
static LayerBit *pbuf;
LayerBit *l1buf, *l2buf;
LayerBit *buf;
static SDL_Rect screenRect, layerRect, layerClearRect;
static int pitch;

Uint8 *keys;
SDL_Joystick *stick = NULL;

// Initialize palletes.
static void initPalette() {
    int i;
    for ( i=0 ; i<256 ; i++ ) {
        color[i].r = color[i].r*brightness/256;
        color[i].g = color[i].g*brightness/256;
        color[i].b = color[i].b*brightness/256;
    }
    SDL_SetColors(video, color, 0, 256);
    SDL_SetColors(layer, color, 0, 256);
}

static int lyrSize;

static void makeSmokeBuf() {
    int x, y, mx, my;
    lyrSize = sizeof(LayerBit)*pitch*SCREEN_HEIGHT;
    if ( NULL == (smokeBuf = (LayerBit**)malloc(sizeof(LayerBit*)*pitch*SCREEN_HEIGHT)) ) {
        fprintf(stderr, "Couldn't malloc smokeBuf.");
        exit(1);
    }

    if (NULL == (pbuf  = (LayerBit*)malloc(lyrSize+sizeof(LayerBit))) ||
            NULL == (l1buf = (LayerBit*)malloc(lyrSize+sizeof(LayerBit))) ||
            NULL == (l2buf = (LayerBit*)malloc(lyrSize+sizeof(LayerBit))) ) {
        fprintf(stderr, "Couldn't malloc buffer.");
        exit(1);
    }

    pbuf[pitch*SCREEN_HEIGHT] = 0;
    for ( y=0 ; y<SCREEN_HEIGHT ; y++ ) {
        for ( x=0 ; x<SCREEN_WIDTH ; x++ ) {
            mx = x + sctbl[(x*8)&(DIV-1)]/128;
            my = y + sctbl[(y*8)&(DIV-1)]/128;
            if ( mx < 0 || mx >= SCREEN_WIDTH || my < 0 || my >= SCREEN_HEIGHT ) {
                smokeBuf[x+y*pitch] = &(pbuf[pitch*SCREEN_HEIGHT]);
            } else {
                smokeBuf[x+y*pitch] = &(pbuf[mx+my*pitch]);
            }
        }
    }
}

void initSDL(int window) {
    Uint8 videoBpp;
    Uint32 videoFlags;
    SDL_PixelFormat *pfrm;

    if ( SDL_Init(SDL_INIT_VIDEO | SDL_INIT_JOYSTICK) < 0 ) {
        fprintf(stderr, "Unable to initialize SDL: %s\n", SDL_GetError());
        exit(1);
    }
    atexit(SDL_Quit);

    videoBpp = BPP;
    videoFlags = SDL_DOUBLEBUF | SDL_HWSURFACE | SDL_HWPALETTE;
    if ( !window ) videoFlags |= SDL_FULLSCREEN;

    if ( (video = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, videoBpp, videoFlags)) == NULL ) {
        fprintf(stderr, "Unable to create SDL screen: %s\n", SDL_GetError());
        SDL_Quit();
        exit(1);
    }

    screenRect.x = screenRect.y = 0;
    screenRect.w = SCREEN_WIDTH; screenRect.h = SCREEN_HEIGHT;
    pfrm = video->format;
    if ( NULL == ( layer = SDL_CreateRGBSurface
                (SDL_SWSURFACE, SCREEN_WIDTH, SCREEN_HEIGHT, videoBpp,
                 pfrm->Rmask, pfrm->Gmask, pfrm->Bmask, pfrm->Amask)) ) {
        fprintf(stderr, "Couldn't create surface: %s\n", SDL_GetError());
        exit(1);
    }

    layerRect.x = 0;
    layerRect.y = 0;
    layerRect.w = SCREEN_WIDTH;
    layerRect.h = SCREEN_HEIGHT;
    layerClearRect.x = layerClearRect.y = 0;
    layerClearRect.w = SCREEN_WIDTH;
    layerClearRect.h = SCREEN_HEIGHT;

    pitch = layer->pitch/(videoBpp/8);
    buf = (LayerBit*)layer->pixels;

    initPalette();
    makeSmokeBuf();

    stick = SDL_JoystickOpen(0);

    SDL_ShowCursor(SDL_ENABLE);
    //SDL_WM_GrabInput(SDL_GRAB_ON);
}

void closeSDL() {
    SDL_ShowCursor(SDL_ENABLE);
}

void blendScreen() {
    int i;
    for ( i = lyrSize-1 ; i >= 0 ; i-- ) {
        buf[i] = colorAlp[l1buf[i]][l2buf[i]];
    }
}

void flipScreen() {
    SDL_BlitSurface(layer, NULL, video, &layerRect);
    SDL_Flip(video);
}

void clearScreen() {
    SDL_FillRect(layer, &layerClearRect, 0);
}

void smokeScreen() {
    int i;
    memcpy(pbuf, l2buf, lyrSize);
    for ( i = lyrSize-1 ; i >= 0 ; i-- ) {
        l1buf[i] = colorDfs[l1buf[i]];
        l2buf[i] = colorDfs[*(smokeBuf[i])];
    }
}

void drawLine(int x1, int y1, int x2, int y2, LayerBit color, int width, LayerBit *buf) {
    int lx, ly, ax, ay, x, y, ptr, i, j;
    int xMax, yMax;

    lx = absN(x2 - x1);
    ly = absN(y2 - y1);

    if ( lx < ly ) {
        x1 -= width>>1; x2 -= width>>1;
    } else {
        y1 -= width>>1; y2 -= width>>1;
    }
    xMax = SCREEN_WIDTH-1; yMax = SCREEN_HEIGHT-1;

    if ( x1 < 0 ) {
        if ( x2 < 0 ) return;
        y1 = (y1-y2)*x2/(x2-x1)+y2;
        x1 = 0;
    } else if ( x2 < 0 ) {
        y2 = (y2-y1)*x1/(x1-x2)+y1;
        x2 = 0;
    }
    if ( x1 > xMax ) {
        if ( x2 > xMax ) return;
        y1 = (y1-y2)*(x2-xMax)/(x2-x1)+y2;
        x1 = xMax;
    } else if ( x2 > xMax ) {
        y2 = (y2-y1)*(x1-xMax)/(x1-x2)+y1;
        x2 = xMax;
    }
    if ( y1 < 0 ) {
        if ( y2 < 0 ) return;
        x1 = (x1-x2)*y2/(y2-y1)+x2;
        y1 = 0;
    } else if ( y2 < 0 ) {
        x2 = (x2-x1)*y1/(y1-y2)+x1;
        y2 = 0;
    }
    if ( y1 > yMax ) {
        if ( y2 > yMax ) return;
        x1 = (x1-x2)*(y2-yMax)/(y2-y1)+x2;
        y1 = yMax;
    } else if ( y2 > yMax ) {
        x2 = (x2-x1)*(y1-yMax)/(y1-y2)+x1;
        y2 = yMax;
    }

    lx = abs(x2 - x1);
    ly = abs(y2 - y1);

    if ( lx < ly ) {
        if ( ly == 0 ) ly++;
        ax = ((x2 - x1)<<8) / ly;
        ay = ((y2 - y1)>>8) | 1;
        x  = x1<<8;
        y  = y1;
        for ( i=ly ; i>0 ; i--, x+=ax, y+=ay ){
            ptr = y*pitch + (x>>8);
            for ( j=width ; j>0 ; j--, ptr++ ) {
                buf[ptr] = color;
            }
        }
    } else {
        if ( lx == 0 ) lx++;
        ay = ((y2 - y1)<<8) / lx;
        ax = ((x2 - x1)>>8) | 1;
        x  = x1;
        y  = y1<<8;
        for ( i=lx ; i>0 ; i--, x+=ax, y+=ay ) {
            ptr = (y>>8)*pitch + x;
            for ( j=width ; j>0 ; j--, ptr+=pitch ) {
                buf[ptr] = color;
            }
        }
    }
}

void drawThickLine(int x1, int y1, int x2, int y2,
		   LayerBit color1, LayerBit color2, int width) {
    int lx, ly, ax, ay, x, y, ptr, i, j;
    int xMax, yMax;
    int width1;

    lx = abs(x2 - x1);
    ly = abs(y2 - y1);
    if ( lx < ly ) {
        x1 -= width>>1; x2 -= width>>1;
    } else {
        y1 -= width>>1; y2 -= width>>1;
    }
    xMax = SCREEN_WIDTH; yMax = SCREEN_HEIGHT;

    if ( x1 < 0 ) {
        if ( x2 < 0 ) return;
        y1 = (y1-y2)*x2/(x2-x1)+y2;
        x1 = 0;
    } else if ( x2 < 0 ) {
        y2 = (y2-y1)*x1/(x1-x2)+y1;
        x2 = 0;
    }

    if ( x1 > xMax ) {
        if ( x2 > xMax ) return;
        y1 = (y1-y2)*(x2-xMax)/(x2-x1)+y2;
        x1 = xMax;
    } else if ( x2 > xMax ) {
        y2 = (y2-y1)*(x1-xMax)/(x1-x2)+y1;
        x2 = xMax;
    }

    if ( y1 < 0 ) {
        if ( y2 < 0 ) return;
        x1 = (x1-x2)*y2/(y2-y1)+x2;
        y1 = 0;
    } else if ( y2 < 0 ) {
        x2 = (x2-x1)*y1/(y1-y2)+x1;
        y2 = 0;
    }

    if ( y1 > yMax ) {
        if ( y2 > yMax ) return;
        x1 = (x1-x2)*(y2-yMax)/(y2-y1)+x2;
        y1 = yMax;
    } else if ( y2 > yMax ) {
        x2 = (x2-x1)*(y1-yMax)/(y1-y2)+x1;
        y2 = yMax;
    }

    lx = abs(x2 - x1);
    ly = abs(y2 - y1);
    width1 = width - 2;

    if ( lx < ly ) {
        if ( ly == 0 ) ly++;
        ax = ((x2 - x1)<<8) / ly;
        ay = ((y2 - y1)>>8) | 1;
        x  = x1<<8;
        y  = y1;
        ptr = y*pitch + (x>>8) + 1;
        memset(&(buf[ptr]), color2, width1);
        x += ax; y += ay;
        for ( i = ly-1 ; i > 1 ; i--, x+=ax, y+=ay ){
            ptr = y*pitch + (x>>8);
            buf[ptr] = color2; ptr++;
            memset(&(buf[ptr]), color1, width1); ptr += width1;
            buf[ptr] = color2;
        }
        ptr = y*pitch + (x>>8) + 1;
        memset(&(buf[ptr]), color2, width1);
    } else {
        if ( lx == 0 ) lx++;
        ay = ((y2 - y1)<<8) / lx;
        ax = ((x2 - x1)>>8) | 1;
        x  = x1;
        y  = y1<<8;
        ptr = ((y>>8)+1)*pitch + x;
        for ( j=width1 ; j>0 ; j--, ptr+=pitch ) {
            buf[ptr] = color2;
        }
        x += ax; y += ay;
        for ( i=lx-1 ; i>1 ; i--, x+=ax, y+=ay ) {
            ptr = (y>>8)*pitch + x;
            buf[ptr] = color2; ptr += pitch;
            for ( j=width1 ; j>0 ; j--, ptr+=pitch ) {
                buf[ptr] = color1;
            }
            buf[ptr] = color2;
        }
        ptr = ((y>>8)+1)*pitch + x;
        for ( j=width1 ; j>0 ; j--, ptr+=pitch ) {
            buf[ptr] = color2;
        }
    }
}

#define JOYSTICK_AXIS 16384

int getPadState() {
  int x = 0, y = 0;
  int pad = 0;
  if ( stick != NULL ) {
    x = SDL_JoystickGetAxis(stick, 0);
    y = SDL_JoystickGetAxis(stick, 1);
  }
  if ( keys[SDLK_RIGHT] == SDL_PRESSED || keys[SDLK_KP6] == SDL_PRESSED || x > JOYSTICK_AXIS ) {
    pad |= PAD_RIGHT;
  }
  if ( keys[SDLK_LEFT] == SDL_PRESSED || keys[SDLK_KP4] == SDL_PRESSED || x < -JOYSTICK_AXIS ) {
    pad |= PAD_LEFT;
  }
  if ( keys[SDLK_DOWN] == SDL_PRESSED || keys[SDLK_KP2] == SDL_PRESSED || y > JOYSTICK_AXIS ) {
    pad |= PAD_DOWN;
  }
  if ( keys[SDLK_UP] == SDL_PRESSED ||  keys[SDLK_KP8] == SDL_PRESSED || y < -JOYSTICK_AXIS ) {
    pad |= PAD_UP;
  }
  return pad;
}

int buttonReversed = 0;

int getButtonState() {
    int btn = 0;
    int btn1 = 0, btn2 = 0, btn3 = 0, btn4 = 0;
    if ( stick != NULL ) {
        btn1 = SDL_JoystickGetButton(stick, 0);
        btn2 = SDL_JoystickGetButton(stick, 1);
        btn3 = SDL_JoystickGetButton(stick, 2);
        btn4 = SDL_JoystickGetButton(stick, 3);
    }
    if ( keys[SDLK_z] == SDL_PRESSED || btn1 || btn4 ) {
        if ( !buttonReversed ) {
            btn |= PAD_BUTTON1;
        } else {
            btn |= PAD_BUTTON2;
        }
    }
    if ( keys[SDLK_x] == SDL_PRESSED || btn2 || btn3 ) {
        if ( !buttonReversed ) {
            btn |= PAD_BUTTON2;
        } else {
            btn |= PAD_BUTTON1;
        }
    }
    return btn;
}
