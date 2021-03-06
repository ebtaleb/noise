#include "SDL.h"
#include "vector.h"

#define SCREEN_WIDTH 800
#define SCREEN_HEIGHT 600
#define LAYER_WIDTH 400
#define LAYER_HEIGHT 600

#define SCAN_WIDTH 400
#define SCAN_HEIGHT 600
#define SCAN_WIDTH_8 (SCAN_WIDTH<<8)
#define SCAN_HEIGHT_8 (SCAN_HEIGHT<<8)

#define BPP 8
#define LayerBit Uint8

#define PAD_UP 1
#define PAD_DOWN 2
#define PAD_LEFT 4
#define PAD_RIGHT 8
#define PAD_BUTTON1 16
#define PAD_BUTTON2 32

#define DEFAULT_BRIGHTNESS 224

extern int windowMode;
extern LayerBit *l1buf, *l2buf;
extern LayerBit *buf;
extern LayerBit *lpbuf, *rpbuf;
extern Uint8 *keys;
extern SDL_Joystick *stick;
extern int buttonReversed;
extern int brightness;

void initSDL(int window);
void closeSDL();
void blendScreen();
void flipScreen();
void clearScreen();
void clearLPanel();
void clearRPanel();
void smokeScreen();
void drawThickLine(int x1, int y1, int x2, int y2, LayerBit color1, LayerBit color2, int width);
void drawLine(int x1, int y1, int x2, int y2, LayerBit color, int width, LayerBit *buf);
void drawBox(int x, int y, int width, int height,
	     LayerBit color1, LayerBit color2, LayerBit *buf);
void drawBoxPanel(int x, int y, int width, int height,
		  LayerBit color1, LayerBit color2, LayerBit *buf);
void drawSprite(int n, int x, int y);

int getPadState();
int getButtonState();
