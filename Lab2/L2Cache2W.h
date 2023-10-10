#ifndef L2CACHE2W_H
#define L2CACHE2W_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "Cache.h"

void resetTime();

uint32_t getTime();

/****************  RAM memory (byte addressable) ***************/
void accessDRAM(uint32_t, uint8_t *, uint32_t);

/*********************** Cache *************************/
void initCache();

void accessL1(uint32_t, uint8_t *, uint32_t);

void accessL2(uint32_t, uint8_t *, uint32_t);

typedef struct CacheLine {
  uint8_t Valid;
  uint8_t Dirty;
  uint32_t Tag;
  uint32_t Time;
} CacheLine;

typedef struct L1 {
  uint32_t init;
  CacheLine lines[L1_LINE_COUNT];
} L1;

typedef struct L2 {
  uint32_t init;
  CacheLine lines[L2_2W_LINE_COUNT][L2_ASSOCIATIVITY_SET];
} L2;

/*********************** Interfaces *************************/

void read(uint32_t, uint8_t *);

void write(uint32_t, uint8_t *);

#endif
