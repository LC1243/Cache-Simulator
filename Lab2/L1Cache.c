#include "L1Cache.h"

unsigned char L1Cache[L1_CACHE_SIZE];
unsigned char L2Cache[L2_SIZE];
unsigned char DRAM[DRAM_SIZE];
unsigned int time;
L1 SimpleCache;

/**************** Time Manipulation ***************/
void resetTime() { time = 0; }

unsigned int getTime() { return time; }

/****************  RAM memory (byte addressable) ***************/
void accessDRAM(int address, unsigned char *data, int mode) {

  if (address >= DRAM_SIZE - WORD_SIZE + 1)
    exit(-1);

  if (mode == MODE_READ) {
    memcpy(data, &(DRAM[address]), BLOCK_SIZE);
    time += DRAM_READ_TIME;
  }

  if (mode == MODE_WRITE) {
    memcpy(&(DRAM[address]), data, BLOCK_SIZE);
    time += DRAM_WRITE_TIME;
  }
}

/*********************** L1 cache *************************/

void initCache() { SimpleCache.init = 0; }

void accessL1(int address, unsigned char *data, int mode) {

  unsigned int index, Tag, MemAddress;
  unsigned char TempBlock[BLOCK_SIZE];

  /* init cache */
  if (SimpleCache.init == 0) {
    for (int i = 0; i < L1_LINE_COUNT; i++)
      SimpleCache.lines[i].Valid = 0;
    SimpleCache.init = 1;
  }

  unsigned int mask = ((1 << 3) - 1); // maximum index value
  index = address >> 3; // removes the offset
  index = index & mask; // removes the tag (assuming 32 bit address)

  Tag = address >> 6; // removes the offset and index

  MemAddress = address >> 3; // removes offset
  MemAddress = MemAddress << 3; // address of the block in memory (sets offset to 0)

  CacheLine *Line = &SimpleCache.lines[index];

  /* access Cache*/

  if (!Line->Valid || Line->Tag != Tag) {         // if block not present - miss
    accessDRAM(MemAddress, TempBlock, MODE_READ); // get new block from DRAM

    if ((Line->Valid) && (Line->Dirty)) { // line has dirty block
      accessDRAM(MemAddress, &(L1Cache[index * BLOCK_SIZE]),
                 MODE_WRITE); // then write back old block
    }

    memcpy(&(L1Cache[index * BLOCK_SIZE]), TempBlock,
           BLOCK_SIZE); // copy new block to cache line
    Line->Valid = 1;
    Line->Tag = Tag;
    Line->Dirty = 0;
  } // if miss, then replaced with the correct block

  if (mode == MODE_READ) {    // read data from cache line
    if (0 == (address % 8)) { // even word on block
      memcpy(data, &(L1Cache[index * BLOCK_SIZE]), WORD_SIZE);
    } else { // odd word on block
      memcpy(data, &(L1Cache[index * BLOCK_SIZE + WORD_SIZE]), WORD_SIZE);
    }
    time += L1_READ_TIME;
  }

  if (mode == MODE_WRITE) { // write data from cache line
    if (!(address % 8)) {   // even word on block
      memcpy(&(L1Cache[index * BLOCK_SIZE]), data, WORD_SIZE);
    } else { // odd word on block
      memcpy(&(L1Cache[index * BLOCK_SIZE + WORD_SIZE]), data, WORD_SIZE);
    }
    time += L1_WRITE_TIME;
    Line->Dirty = 1;
  }
}

void read(int address, unsigned char *data) {
  accessL1(address, data, MODE_READ);
}

void write(int address, unsigned char *data) {
  accessL1(address, data, MODE_WRITE);
}
