#include "L1Cache.h"

uint8_t L1Cache[L1_SIZE];
uint8_t L2Cache[L2_SIZE];
uint8_t DRAM[DRAM_SIZE];
uint32_t time;
L1 L1_Info;

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

void initCache() { L1_Info.init = 0; }

void accessL1(uint32_t address, uint8_t *data, uint32_t mode) {

  uint32_t index, Tag, MemAddress, offset;
  uint8_t TempBlock[BLOCK_SIZE];

  /* init cache */
  if (L1_Info.init == 0) {
    for (int i = 0; i < L1_LINE_COUNT; i++) {
      L1_Info.lines[i].Valid = 0;
      L1_Info.lines[i].Dirty = 0;
      L1_Info.lines[i].Tag = 0;
      for (int j = 0; j < BLOCK_SIZE; j+= WORD_SIZE) 
        L1Cache[i * BLOCK_SIZE + j] = 0; // Sets all L1Cache data to 0
    }
        
    L1_Info.init = 1;
  }

  offset = address << 26;
  offset = offset >> 26;

  unsigned int mask = ((1 << 8) - 1); // maximum index value
  index = address >> 6; // removes the offset
  index = index & mask; // removes the tag (assuming 32 bit address)

  Tag = address >> 14; // removes the offset and index

  MemAddress = address >> 6; // removes offset
  MemAddress = MemAddress << 6; // address of the block in memory (sets offset to 0)

  CacheLine *Line = &L1_Info.lines[index];
  /* access Cache*/

  if (!Line->Valid || Line->Tag != Tag) {         // if block not present - miss
    accessDRAM(MemAddress, TempBlock, MODE_READ); // get new block from DRAM

    if ((Line->Valid) && (Line->Dirty)) { // line has dirty block
      MemAddress = Line->Tag << 6;        // get address of the block in memory
      accessDRAM(MemAddress, &(L1Cache[index * BLOCK_SIZE]), MODE_WRITE); // then write back old block
    }

    memcpy(&(L1Cache[index * BLOCK_SIZE]), TempBlock,
           BLOCK_SIZE); // copy new block to cache line
    Line->Valid = 1;
    Line->Tag = Tag;
    Line->Dirty = 0;
  } // if miss, then replaced with the correct block

  if (mode == MODE_READ) {    // read data from cache line
    memcpy(data, &(L1Cache[index * BLOCK_SIZE + offset]), WORD_SIZE);
    time += L1_READ_TIME;
    Line->Valid = 1;
    Line->Tag = Tag;
    Line->Dirty = 0;
  }

  if (mode == MODE_WRITE) { // write data from cache line
    memcpy(&(L1Cache[index * BLOCK_SIZE + offset]), data, WORD_SIZE);
    time += L1_WRITE_TIME;
    Line->Valid = 1;
    Line->Tag = Tag;
    Line->Dirty = 1;
  }
}

void read(uint32_t address, uint8_t *data) {
  accessL1(address, data, MODE_READ);
}

void write(uint32_t address, uint8_t *data) {
  accessL1(address, data, MODE_WRITE);
}
