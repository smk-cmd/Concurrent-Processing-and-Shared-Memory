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


#define NUM_PELLETS 2
#define TIMEOUT 30

void sigchild_handler(int sig);
void print_mill();

int reaped_children = 0;

struct shmseg *shmp;

int main(int argc, char* argv[]) {
  double time_spent = 0.0;
  clock_t begin = clock();
  int shmid;


  printf("Swim Mill Simulation Starting\n");

  // register a handler for SIGCHLD
  signal(SIGCHLD, sigchild_handler);

  // ******** START SHARED MEMORY SETUP ***************
  // allocates a shared memory ID
  // 0666 is the permission settings, IPC_CREAT creates it if it doesn't already exist
  //printf("size of my struct: %lu\n", sizeof(struct shmseg));
  // trial and error on differrent types of errors to see why I was getting shmget error: cannot allocate memory

  shmid = shmget(SHM_KEY, sizeof(struct shmseg), IPC_CREAT | 0666);
  if (shmid == -1) {
    if(errno == EACCES) {
      printf("shmget EACCESS error: %s\n", strerror(errno));
    }
    else if(errno == EEXIST) {
      printf("shmget EEXIST error: %s\n", strerror(errno));
    }
    else if(errno == EINVAL) {
      printf("shmget EINVAL error: %s\n", strerror(errno));
    }
    else if(errno == ENFILE) {
      printf("shmget ENFILE error: %s\n", strerror(errno));
    }
    else if(errno == ENOENT) {
      printf("shmget ENOENT error: %s\n", strerror(errno));
    }
    else if(errno == ENOMEM) {
      printf("shmget ENOMEM error: %s\n", strerror(errno));
    }
    else if(errno == ENOSPC) {
      printf("shmget ENOSPC error: %s\n", strerror(errno));
    }
    else if(errno == EPERM) {
      printf("shmget EPERM error: %s\n", strerror(errno));
    }
    else {
      printf("shmget UNKNOWN error: %s\n", strerror(errno));
    }

    return 1;
  }

  // Will use shmat because its job is to attach the memory segment to get a pointer to it.
  // will get the address space of the process or else will exit (on error).
  shmp = shmat(shmid, NULL, 0);
  if (shmp == (void *) -1) {
    perror("shmat error");
    return 1;
  }

  shmp->rows = NUM_OF_ROWS;
  shmp->cols = NUM_OF_COLUMNS;

  // clear the mill
  for(int r = 0; r < NUM_OF_ROWS; r++) {
    for(int c = 0; c < NUM_OF_COLUMNS; c++) {
      shmp->stream[r][c] = ' ';
    }
  }
   // ********** END SHARED MEMORY SETUP ***********
  
   printf("Swim mill rows = %d cols = %d\n", shmp->rows, shmp->cols);

  // spawn NUM_PELLETS processes
  for(int i = 0; i < NUM_PELLETS; i++) {
    pid_t pid = fork();
    // child
    if(pid == 0) {
        // start a pellet process
        execv("./pellet", argv);
        printf("ERROR, execv failed\n");
    }
  }

  // This will spawn the fish or else it will result in an error
  pid_t fish_pid = fork();
  if(fish_pid == 0) {
    execv("./fish", argv);
    printf("ERROR, execv failed\n");
  }
  
  // wait for all the children to complete
  while(reaped_children < NUM_PELLETS) {

    time_spent = (clock() - begin)/CLOCKS_PER_SEC;
    if(time_spent > TIMEOUT) {
      // terminate the fish child process because it ran past the allowed time of 30 seconds
      kill(fish_pid, SIGKILL);

      // detach from and mark the shared memory for destruction
      if (shmdt(shmp) == -1) {
        perror("shmdt");
        return 1;
      }

      // shmctl is the "controller"
      // we will start to terminate due to time limit using IPC_RMID - which will mark the segment to be destroyed
      if (shmctl(shmid, IPC_RMID, 0) == -1) {
        perror("shmctl");
        return 1;
      }
      
      // will print message after everything has been terminated off due to time limit
      printf("Swim Mill Simulation exceeded allowed time of %d seconds, goodbye!\n", TIMEOUT);
      exit(0);
    }
  }

  // terminate the fish child process
  kill(fish_pid, SIGKILL);
  printf("Swim Mill Simulation complete after %f seconds, goodbye!\n", time_spent);

  return 0;
}

// child process stops and prints its pid 
void sigchild_handler(int sig) {
  int status;
  pid_t pid = waitpid(-1, &status, 0);
  printf("child %d ended\n", pid);
  reaped_children++;
}
