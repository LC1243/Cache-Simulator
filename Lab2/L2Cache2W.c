#include "L2Cache2W.h"

uint8_t L1Cache[L1_SIZE];
uint8_t L2Cache[L2_SIZE];
uint8_t DRAM[DRAM_SIZE];
uint32_t time;
L1 L1_Info;
L2 L2_Info;

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

void initCache() { L1_Info.init = 0; L2_Info.init = 0; }

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
    accessL2(MemAddress, TempBlock, MODE_READ); // get new block from DRAM

    if ((Line->Valid) && (Line->Dirty)) { // line has dirty block
      MemAddress = Line->Tag << 6;        // get address of the block in memory
      accessL2(MemAddress, &(L1Cache[index * BLOCK_SIZE]), MODE_WRITE); // then write back old block
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

/*********************** L2 cache *************************/

void accessL2(uint32_t address, uint8_t *data, uint32_t mode) {

  uint32_t index, Tag, MemAddress, offset;
  uint8_t TempBlock[BLOCK_SIZE];

  /* init cache */
  if (L2_Info.init == 0) {
    for (int i = 0; i < L2_2W_LINE_COUNT; i++) {
        for(int k = 0; k < L2_ASSOCIATIVITY_SET; k++) {
            L2_Info.lines[i][k].Valid = 0;
            L2_Info.lines[i][k].Dirty = 0;
            L2_Info.lines[i][k].Tag = 0;
            L2_Info.lines[i][k].Time = 0;
        }
        for (int j = 0; j < 2 * BLOCK_SIZE; j+= WORD_SIZE) 
            L2Cache[i * BLOCK_SIZE + j] = 0; // Sets all L2Cache data to 0
    }
        
    L2_Info.init = 1;
  }

  offset = address << 26;
  offset = offset >> 26;

  unsigned int mask = ((1 << 8) - 1); // maximum index value
  index = address >> 6; // removes the offset
  index = index & mask; // removes the tag (assuming 32 bit address)

  Tag = address >> 14; // removes the offset and index

  MemAddress = address >> 6; // removes offset
  MemAddress = MemAddress << 6; // address of the block in memory (sets offset to 0)

  /* access Cache*/

  uint8_t found = 0;
  int i = 0;
  while (i < L2_ASSOCIATIVITY_SET) {
    if (L2_Info.lines[index][i].Tag == Tag && L2_Info.lines[index][i].Valid == 1) {
        found = 1;
        break;
    } else
        i++;
  }

  if (!found) {
    int blocks_occupied = 0;
    
    while (blocks_occupied < L2_ASSOCIATIVITY_SET && L2_Info.lines[index][blocks_occupied].Valid)
        blocks_occupied++;

    if (blocks_occupied == L2_ASSOCIATIVITY_SET) { // All blocks are occupied
        i = 0;
        uint32_t min = L2_Info.lines[index][i].Time;

        for (int j = 1; j < L2_ASSOCIATIVITY_SET; j++) // Gets the LRU block's index
            if (L2_Info.lines[index][j].Time < min) {
                min = L2_Info.lines[index][j].Time;
                i = j;
            }
    }
    if (L2_Info.lines[index][i].Valid && L2_Info.lines[index][i].Dirty) {
        MemAddress = L2_Info.lines[index][i].Tag << 6;        // get address of the block in memory
        accessDRAM(MemAddress, &(L2Cache[index * 2 * BLOCK_SIZE + (i * BLOCK_SIZE)]), MODE_WRITE); // then write back old block
        //L2Cache[] = 0
    }

    accessDRAM(MemAddress - offset , &(L2Cache[index * 2 * BLOCK_SIZE + (i * BLOCK_SIZE)]), MODE_READ);

    L2_Info.lines[index][i].Valid = 1;
    L2_Info.lines[index][i].Tag = Tag;
    L2_Info.lines[index][i].Dirty = 0;
    L2_Info.lines[index][i].Time = time;

    if (mode == MODE_READ) {
      memcpy(data, &(L2Cache[index * 2 * BLOCK_SIZE + (i * BLOCK_SIZE)]), BLOCK_SIZE);
      time += L2_READ_TIME;
    }
  } else {

  if (mode == MODE_READ) {    // read data from cache line
    memcpy(data, &(L2Cache[index * 2 * BLOCK_SIZE + (i * BLOCK_SIZE)]), BLOCK_SIZE);
    time += L2_READ_TIME;
    L2_Info.lines[index][i].Valid = 1;
    L2_Info.lines[index][i].Tag = Tag;
    L2_Info.lines[index][i].Dirty = 0;
    L2_Info.lines[index][i].Time = time;
  }

  if (mode == MODE_WRITE) { // write data from cache line
    memcpy(&(L2Cache[index * 2 * BLOCK_SIZE + (i * BLOCK_SIZE)]), data, BLOCK_SIZE);
    time += L2_WRITE_TIME;
    L2_Info.lines[index][i].Valid = 1;
    L2_Info.lines[index][i].Tag = Tag;
    L2_Info.lines[index][i].Dirty = 1;
    L2_Info.lines[index][i].Time = time;
  }
  }
}


void read(uint32_t address, uint8_t *data) {
  accessL1(address, data, MODE_READ);
}

void write(uint32_t address, uint8_t *data) {
  accessL1(address, data, MODE_WRITE);
}
