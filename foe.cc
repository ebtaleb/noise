#include "barragemanager.h"
#include "foe.h"

extern "C" {
#include "SDL.h"
#include <stdlib.h>
#include <stdio.h>

#include "noise.h"
#include "screen.h"
#include "vector.h"
#include "degutil.h"
#include "brgmng_mtd.h"
}

#define FOE_MAX 1024
#define FOE_TYPE_MAX 4

static Foe foe[FOE_MAX];
int foeCnt, enNum[FOE_TYPE_MAX];

static void removeFoeForcedNoDeleteCmd(Foe *fe) {
    if ( fe->spc == FOE ) {
        foeCnt--; enNum[fe->type]--;
    }
    fe->spc = NOT_EXIST;
}

static void removeFoeForced(Foe *fe) {
    removeFoeForcedNoDeleteCmd(fe);
    if ( fe->cmd ) {
        delete fe->cmd;
        fe->cmd = NULL;
    }
}

void removeFoe(Foe *fe) {
    if ( fe->type == BOSS_TYPE ) return;
    removeFoeForcedNoDeleteCmd(fe);
}

void initFoes() {
    int i = 0;
    for ( i=0 ; i<FOE_MAX ; i++ ) {
        removeFoeForced(&(foe[i]));
    }

    foeCnt = 0;
    for ( i=0 ; i<FOE_TYPE_MAX ; i++ ) {
        enNum[i] = 0;
    }
}

void closeFoes() {
    int i = 0;
    for ( i=0 ; i<FOE_MAX ; i++ ) {
        if ( foe[i].cmd ) delete foe[i].cmd;
    }
}

static int foeIdx = FOE_MAX;

static Foe* getNextFoe() {
    int i;
    for ( i=0 ; i<FOE_MAX ; i++ ) {
        foeIdx--;
        if ( foeIdx < 0 ) foeIdx = FOE_MAX-1;
        if ( foe[i].spc == NOT_EXIST ) break;
    }

    if ( i >= FOE_MAX ) return NULL;

    return &(foe[i]);
}

Foe* addFoe(int x, int y, double rank, int d, int spd, int type, int shield,
	    BulletMLParser *parser) {
    Foe *fe = getNextFoe();
    if ( !fe ) return NULL;

    fe->parser = parser;

    fe->cmd = new FoeCommand(parser, fe);

    fe->pos.x = x; fe->pos.y = y;
    fe->spos = fe->ppos = fe->pos;
    fe->vel.x = fe->vel.y = 0;
    fe->rank = rank;
    fe->d = d; fe->spd = spd;
    fe->spc = FOE;
    fe->type = type;
    fe->shield = shield;
    fe->cnt = 0;
    fe->color = 0;
    fe->hit = 0;

    return fe;
}

Foe* addFoeBossActiveBullet(int x, int y, double rank,
			    int d, int spd, BulletMLParser *parser) {
    Foe *fe = addFoe(x, y, rank, d, spd, BOSS_TYPE, 0, parser);
    if ( !fe ) return NULL;
    foeCnt--; enNum[BOSS_TYPE]--;
    fe->spc = BOSS_ACTIVE_BULLET;
    return fe;
}

void addFoeActiveBullet(Vector *pos, double rank,
			int d, int spd, int color, BulletMLState *state) {
    Foe *fe = getNextFoe();
    if ( !fe ) return;
    fe->cmd = new FoeCommand(state, fe);
    fe->spos = fe->ppos = fe->pos = *pos;
    fe->vel.x = fe->vel.y = 0;
    fe->rank = rank;
    fe->d = d; fe->spd = spd;
    fe->spc = ACTIVE_BULLET;
    fe->type = 0;
    fe->cnt = 0;
    fe->color = color;
}

void addFoeNormalBullet(Vector *pos, double rank, int d, int spd, int color) {
    Foe *fe = getNextFoe();
    if ( !fe ) return;
    fe->cmd = NULL;
    fe->spos = fe->ppos = fe->pos = *pos;
    fe->vel.x = fe->vel.y = 0;
    fe->rank = rank;
    fe->d = d; fe->spd = spd;
    fe->spc = BULLET;
    fe->type = 0;
    fe->cnt = 0;
    fe->color = color;
}


#define BULLET_WIPE_WIDTH 7200

#define SHIP_HIT_WIDTH 512*512

int processSpeedDownBulletsNum = DEFAULT_SPEED_DOWN_BULLETS_NUM;
int nowait = 0;

void moveFoes() {
    int i = 0;
    Foe *fe;
    int foeNum = 0;
    int mx, my;
    int wl;
    Vector bmv, sofs;
    float ht, hd, inab, inaa;

    for ( i=0 ; i<FOE_MAX ; i++ ) {
        if ( foe[i].spc == NOT_EXIST ) continue;
        fe = &(foe[i]);
        if ( fe->cmd ) {
            if ( fe->type == BOSS_TYPE ) {
                if ( fe->cmd->isEnd() ) {
                    delete fe->cmd;
                    fe->cmd = new FoeCommand(fe->parser, fe);
                }
            }
            fe->cmd->run();
            if ( fe->spc == NOT_EXIST ) {
                if ( fe->cmd ) {
                    delete fe->cmd;
                    fe->cmd = NULL;
                }
                continue;
            }
        }
        mx =  ((sctbl[fe->d % 1280]*fe->spd)>>8) + fe->vel.x;
        my = -((sctbl[(fe->d+256) % 1280]*fe->spd)>>8) + fe->vel.y;
        fe->pos.x += mx;
        fe->pos.y += my;
        fe->mv.x = mx;
        fe->mv.y = my;
        wl = 2;
        if ( fe->cnt < 4 ) wl = 0;
        else if ( fe->cnt < 8 ) wl = 1;
        fe->ppos.x = fe->pos.x - (mx<<wl);
        fe->ppos.y = fe->pos.y - (my<<wl);
        fe->cnt++;

        if ( fe->spc == FOE ) {
            fe->hit = 0;
        } else {
            // Check if the bullet hits the ship.
            bmv = fe->pos;
            vctSub(&bmv, &(fe->ppos));
            inaa = vctInnerProduct(&bmv, &bmv);
            if ( inaa > 1.0f ) {
                vctSub(&sofs, &(fe->ppos));
                inab = vctInnerProduct(&bmv, &sofs);
                ht =  inab / inaa;
                if ( ht > 0.0f && ht < 1.0f ) {
                    hd = vctInnerProduct(&sofs, &sofs) - inab*inab/inaa/inaa;
                    if ( hd >= 0 && hd < SHIP_HIT_WIDTH ) {
                    }
                }
            }
        }
        if ( fe->ppos.x < 0 || fe->ppos.x >= SCAN_WIDTH_8 ||
                fe->ppos.y < 0 || fe->ppos.y >= SCAN_HEIGHT_8 ) {
            removeFoeForced(fe);
            continue;
        }
        foeNum++;
    }

    // A game speed becomes slow as many bullets appears.
    interval = INTERVAL_BASE;
}

#define BULLET_COLOR_NUM 3

static int bulletColor[BULLET_COLOR_NUM][2] = {
    {16*14-1, 16*2-1}, {16*16-1, 16*4-1}, {16*12-1, 16*6-1},
};

#define BULLET_WIDTH 6

void drawBullets() {
    int i;
    Foe *fe;
    int x, y, px, py;
    int bc;
    for ( i=0 ; i<FOE_MAX ; i++ ) {
        if ( foe[i].spc == NOT_EXIST || foe[i].spc == FOE ) continue;
        fe = &(foe[i]);
        x = (fe->pos.x/SCAN_WIDTH*LAYER_WIDTH)>>8;
        y = (fe->pos.y/SCAN_HEIGHT*LAYER_HEIGHT)>>8;
        px = (fe->ppos.x/SCAN_WIDTH*LAYER_WIDTH)>>8;
        py = (fe->ppos.y/SCAN_HEIGHT*LAYER_HEIGHT)>>8;
        bc = fe->color%BULLET_COLOR_NUM;
        drawThickLine(x, y, px, py, bulletColor[bc][0], bulletColor[bc][1], BULLET_WIDTH);
    }
}
