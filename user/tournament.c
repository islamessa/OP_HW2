#include "kernel/types.h"
#include "user/user.h"

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(2, "Usage: %s <num_processes>\n", argv[0]);
    exit(1);
  }

  int num_procs = atoi(argv[1]);
  int tid = tournament_create(num_procs);

  if (tid < 0) {
    fprintf(2, "Error: tournament_create failed\n");
    exit(1);
  }

  // Attempt to enter the critical section
  if (tournament_acquire() < 0) {
    fprintf(2, "Error: tournament_acquire failed\n");
    exit(1);
  }

  // Inside critical section
  printf("Process %d (PID %d) is in the critical section\n", tid, getpid());

  // Optional: slow down to visualize output better
  // sleep(1);

  if (tournament_release() < 0) {
    fprintf(2, "Error: tournament_release failed\n");
    exit(1);
  }

  // Parent process waits for all children before cleanup
  if (tid == 0) {
    for (int i = 1; i < num_procs; i++) {
      wait(0);
    }

    printf("Cleaning up tournament lock structure...\n");
    if (tournament_destroy() < 0) {
      fprintf(2, "Error: tournament_destroy failed\n");
      exit(1);
    }
  }

  exit(0);
}
