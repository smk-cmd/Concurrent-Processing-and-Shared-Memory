#include <stdlib.h> 
#include <time.h> 
#include <unistd.h> 
#include <signal.h> 
#include <stdio.h> 
#include <sys/types.h> 
#include <sys/ipc.h> 
#include <sys/shm.h> 

#include "shared_mem.h"


int main(int argc, char* argv[]) {
  time_t t;

  /* Intializes random number generator */
  srand((unsigned) time(&t) + getpid());
    // ******* SHARED MEMORY SETUP **********
  int shmid; // shared memory id
  struct shmseg *shmp; // structure consisting of max number of memory segments
  // shared memory id will equal to shmget which will allocate memory with certain permissions 
  shmid = shmget(SHM_KEY, sizeof(struct shmseg), IPC_CREAT | 0666);
  if (shmid == -1) {
    perror("Shared memory");
    return 1;
  }

  // Will use shmat because its job is to attach the memory segment to get a pointer to it.
  // will get the address space of the process or else will exit (on error).
  shmp = shmat(shmid, NULL, 0);
  if (shmp == (void *) -1) {
    perror("Shared memory attach");
    return 1;
  }
  // ******* END SHARED MEMORY SETUP *********
  
  int pellet_row = rand() % shmp->rows; // generates random rows of where the pellet can be placed
  int pellet_col = rand() % shmp->cols; // generates random columns of where the pellet can be placed

  printf("Pellet process %d placing pellet at [%d][%d]\n", getpid(), pellet_row, pellet_col);

  shmp->stream[pellet_row][pellet_col] = 'P';

  // Pellet process running and completing
  printf("Pellet process %d started\n", getpid());
  while(1) {
    sleep(1); // sleep for 1 seconds
    shmp->stream[pellet_row][pellet_col] = ' ';
    pellet_row++; //increment pellet row
    if(pellet_row == NUM_OF_ROWS) {
      printf("Pellet is at the end of the stream, process %d ending\n", getpid());
      // detach from the shared memory
      if (shmdt(shmp) == -1) {
        perror("shmdt");
        return 1;
      }
      exit(0);
    }
    shmp->stream[pellet_row][pellet_col] = 'P';
  }
  return 0;
}
