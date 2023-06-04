#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>


#include "shared_mem.h"

struct shmseg *shmp; // structure consisting of max number of memory segments 


// printing out the mill
void print_mill(struct shmseg* shmp) {
  printf("*******************\n");
  for(int r = 0; r < NUM_OF_ROWS; r++) {
    for(int c = 0; c < NUM_OF_COLUMNS; c++) {
        
      printf("%c   ~", shmp->stream[r][c]);
      
    }
    printf("\n");
  }
}

// a function where the fish is done swimming so the whole process will be terminated
void sigkill_handler(int sig) {
  printf("The fish is done swimming, goodbye!\n");
  // detach from the shared memory
  if (shmdt(shmp) == -1) {
    perror("shmdt");
  }
  fflush(stdout);
  exit(0);
}


int main(int argc, char* argv[]) {
  printf("Fish is swimming in the mill\n");

  // register a handler for SIGKILL to detach from shared memory
  // after the simulation is over
  signal(SIGKILL, sigkill_handler);
  signal(SIGTERM, sigkill_handler);

  // ******* SHARED MEMORY SETUP **********
  int shmid; // shared memory id

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


  // ******* END SHARED MEMORY SETUP ********


  // start the fish on the last row in the middle
  shmp->stream[NUM_OF_ROWS-1][NUM_OF_COLUMNS/2] = 'F';

  while(1) {

    print_mill(shmp);
    sleep(1); // sleep for 1s between printing of the mill
  }

}
