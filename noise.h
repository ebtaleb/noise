#define randN(N) (rand()%(N))
#define randNS(N) (rand()%(N<<1)-N)
#define randNS2(N) ((rand()%(N)-(N>>1)) + (rand()%(N)-(N>>1)))
#define absN(a) ((a) < 0 ? - (a) : (a))

#define INTERVAL_BASE 16
#define NOT_EXIST -999999

extern int interval;
extern int tick;

void quitLast();
void initTitle();
void initGame();
