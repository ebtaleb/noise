extern "C" {
#include "SDL.h"
#include <sys/types.h>
#include "noise.h"
#include "degutil.h"
#include "vector.h"
#include "screen.h"
#include "rand.h"
#include "brgmng_mtd.h"
}

#include "barragemanager.h"
#include "foe.h"

#define BARRAGE_PATTERN_MAX 32

static Barrage barragePattern[BARRAGE_PATTERN_MAX];
static Barrage *barrageQueue[BARRAGE_PATTERN_MAX];

static Barrage *barrage[BARRAGE_MAX];

static void readABulletMLFile(const char *path, Barrage brg[]) {
    printf("%s\n", path);
    brg[0].bulletml = new BulletMLParserTinyXML(path);
    brg[0].bulletml->build();
}

static unsigned int rnd;

void initBarragemanager() {
    readABulletMLFile("test.xml", barragePattern);
}

void closeBarragemanager() {
  delete barragePattern[0].bulletml;
}

void initBarrages() {
  rnd = (unsigned int)SDL_GetTicks();
  barragePattern[0].frq = 1;
  barrageQueue[0] = &(barragePattern[0]);
  barrage[0] = &(barragePattern[0]);
}

static int shield[] = {3, 6, 9};
static int appFreq[] = {90, 360, 800};

static Foe *bossBullet;
static int barrageNum = 1;

void addBullets() {
  int x, y, i;
  int type, frq = 0;

  // Scene time control.
  for ( i=0 ; i<barrageNum ; i++ ) {

    frq = appFreq[type]/barrage[i]->frq;
    if ( frq < 2 ) frq = 2;
    if ( (nextRandInt(&rnd)%frq) == 0 ) {
      x = nextRandInt(&rnd)%(SCAN_WIDTH_8*2/3) + (SCAN_WIDTH_8/6);
      y = nextRandInt(&rnd)%(SCAN_HEIGHT_8/6) + (SCAN_HEIGHT_8/10);
      addFoe(x, y, barrage[i]->rank, 512, 0, type, shield[type], barrage[i]->bulletml);
    }
  }
}

#define BOSS_SHIELD 128

void addBossBullet() {
  Foe *bl;
  bossBullet = NULL;

  for ( int i=0 ; i<barrageNum ; i++ ) {
    if ( bossBullet == NULL ) {
      bl = addFoe(SCAN_WIDTH_8/2, SCAN_HEIGHT_8/5, barrage[i]->rank, 512, 0,
		  BOSS_TYPE, BOSS_SHIELD, barrage[i]->bulletml);
      bossBullet = bl;
    } else {
      bl = addFoeBossActiveBullet(SCAN_WIDTH_8/2, SCAN_HEIGHT_8/5, barrage[i]->rank, 512, 0,
				  barrage[i]->bulletml);
    }
  }
}
