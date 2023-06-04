// header file so pellet and fish share the same memory

#ifndef SHARED_MEM_H
#define SHARED_MEM_H

#define NUM_OF_ROWS     10 // number of rows
#define NUM_OF_COLUMNS  10 // numbers of columns 
#define SHM_KEY         0x1234 //input of key size

struct shmseg {     // structure containing max number of shared memory segments per process
  int rows;
  int cols;
  char stream[NUM_OF_ROWS][NUM_OF_COLUMNS];
};

#endif
