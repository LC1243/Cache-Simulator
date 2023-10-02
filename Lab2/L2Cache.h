#include "Cache.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void resetTime();

unsigned int getTime();

/****************  RAM memory (byte addressable) ***************/
void accessDRAM(int, unsigned char *, int);

/*********************** Cache *************************/

void initCacheL1();
void initCacheL2();     
void accessL2(int, unsigned char *, int);
void accessL1(int, unsigned char *, int);

typedef struct CacheLine {
  unsigned char Valid;
  unsigned char Dirty;
  unsigned int Tag;
} CacheLine;

typedef struct L1 {
  int init;
  CacheLine lines[L1_LINE_COUNT];
} L1;

typedef struct L2 {
  int init;
  CacheLine lines[L2_LINE_COUNT];
} L2;

/*********************** Interfaces *************************/

void read(int, unsigned char *);

void write(int, unsigned char *);
