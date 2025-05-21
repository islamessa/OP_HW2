#include "kernel/types.h"
#include "kernel/stat.h"
#include "kernel/fcntl.h"
#include "user/user.h"

// Internal lock storage and metadata
int *lock_ids = 0;
int tournament_index = 0;
int num_levels = 0;
int num_processes = 0;

// Validate number of processes (must be power of two)
int valid_proc_count(int n) {
  return (n == 2 || n == 4 || n == 8 || n == 16);
}

// Initialize tournament lock system
int tournament_create(int processes) {
  if (!valid_proc_count(processes)) {
    fprintf(2, "tournament_create: invalid number of processes\n");
    return -1;
  }

  num_processes = processes;
  num_levels = 0;
  int count = processes;

  // Determine tree depth
  while (count >>= 1)
    num_levels++;

  int internal_locks = processes - 1;
  lock_ids = malloc(sizeof(int) * internal_locks);
  if (!lock_ids)
    return -1;

  // Create Peterson locks at internal nodes
  for (int i = 0; i < internal_locks; i++) {
    lock_ids[i] = peterson_create();
    if (lock_ids[i] < 0)
      return -1;
  }

  // Fork child processes and assign tournament_index
  for (int i = 1; i < processes; i++) {
    int child = fork();
    if (child < 0)
      return -1;
    if (child == 0) {
      tournament_index = i;
      break;
    }
  }

  return tournament_index;
}

// Lock acquisition through the tree
int tournament_acquire(void) {
  if (tournament_index < 0 || lock_ids == 0)
    return -1;

  // Go from leaf to root
  for (int level = num_levels - 1; level >= 0; level--) {
    int bitmask = 1 << (num_levels - level - 1);
    int role = (tournament_index & bitmask) >> (num_levels - level - 1);
    int lock_idx = (tournament_index >> (num_levels - level)) + (1 << level) - 1;

    if (peterson_acquire(lock_ids[lock_idx], role) < 0)
      return -1;
  }

  return 0;
}

// Lock release from root to leaf
int tournament_release(void) {
  if (tournament_index < 0 || lock_ids == 0)
    return -1;

  for (int level = 0; level < num_levels; level++) {
    int bitmask = 1 << (num_levels - level - 1);
    int role = (tournament_index & bitmask) >> (num_levels - level - 1);
    int lock_idx = (tournament_index >> (num_levels - level)) + (1 << level) - 1;

    if (peterson_release(lock_ids[lock_idx], role) < 0)
      return -1;
  }

  return 0;
}

// Clean up all locks and reset state
int tournament_destroy(void) {
  if (!lock_ids || num_processes <= 1)
    return -1;

  for (int i = 0; i < num_processes - 1; i++) {
    if (peterson_destroy(lock_ids[i]) < 0)
      return -1;
  }

  free(lock_ids);
  lock_ids = 0;
  tournament_index = -1;
  num_levels = 0;
  num_processes = 0;

  return 0;
}
