#include "L1Cache.h"

uint8_t L1Cache[L1_SIZE];
uint8_t L2Cache[L2_SIZE];
uint8_t DRAM[DRAM_SIZE];
uint32_t time;
L1 Cache_L1;

/**************** Time Manipulation ***************/
void resetTime() { time = 0; }

uint32_t getTime() { return time; }

/****************  RAM memory (byte addressable) ***************/
void accessDRAM(uint32_t address, uint8_t *data, uint32_t mode) {

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

void initCache() { Cache_L1.init = 0; }

void accessL1(uint32_t address, uint8_t *data, uint32_t mode) {

  uint32_t index, Tag, MemAddress;
  uint8_t TempBlock[BLOCK_SIZE];

  /* init cache */
  if (Cache_L1.init == 0) {
    for (int i = 0; i < L1_LINE_COUNT; i++) {
        Cache_L1.lines[i].Valid = 0;
        Cache_L1.lines[i].Dirty = 0;
        Cache_L1.lines[i].Tag = 0;
    }
        //Pôr data dos blocos a zero??
    Cache_L1.init = 1;
  }


  unsigned int mask = ((1 << 8) - 1); // maximum index value
  index = address >> 6; // removes the offset
  index = index & mask; // removes the tag (assuming 32 bit address)

  Tag = address >> 6; // removes the offset and index

  MemAddress = address >> 6; // removes offset
  MemAddress = MemAddress << 6; // address of the block in memory (sets offset to 0)
  
  CacheLine *Line = &Cache_L1.lines[index];
  /* access Cache*/

  if (!Line->Valid || Line->Tag != Tag) {         // if block not present - miss
    accessDRAM(MemAddress, TempBlock, MODE_READ); // get new block from DRAM

    if ((Line->Valid) && (Line->Dirty)) { // line has dirty block
      MemAddress = Line->Tag << 3;        // get address of the block in memory
      accessDRAM(MemAddress, &(L1Cache[0]), MODE_WRITE); // then write back old block
    }

    memcpy(&(L1Cache[0]), TempBlock,
           BLOCK_SIZE); // copy new block to cache line
    Line->Valid = 1;
    Line->Tag = Tag;
    Line->Dirty = 0;
  } // if miss, then replaced with the correct block

  if (mode == MODE_READ) {    // read data from cache line
    if (0 == (address % 8)) { // even word on block
      memcpy(data, &(L1Cache[0]), WORD_SIZE);
    } else { // odd word on block
      memcpy(data, &(L1Cache[WORD_SIZE]), WORD_SIZE);
    }
    time += L1_READ_TIME;
  }

  if (mode == MODE_WRITE) { // write data from cache line
    if (!(address % 8)) {   // even word on block
      memcpy(&(L1Cache[0]), data, WORD_SIZE);
    } else { // odd word on block
      memcpy(&(L1Cache[WORD_SIZE]), data, WORD_SIZE);
    }
    time += L1_WRITE_TIME;
    Line->Dirty = 1;
  }
}

void read(uint32_t address, uint8_t *data) {
  accessL1(address, data, MODE_READ);
}

void write(uint32_t address, uint8_t *data) {
  accessL1(address, data, MODE_WRITE);
}
