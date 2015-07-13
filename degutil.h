#define DIV 256
#define TAN_TABLE_SIZE 256
#define TABLE_SIZE 256
#define SC_TABLE_SIZE 1024

extern int sctbl[];
extern int sintbl[];
extern int costbl[];

void initDegutil();
int getDeg(int x, int y);
int getDistance(int x, int y);
