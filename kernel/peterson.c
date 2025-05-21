#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "peterson.h"
#include "spinlock.h"
#include "proc.h"

struct petersonlock petersonlocks[NPETERSONLOCKS];

void
petersonlocks_init(void)
{
  for(int i = 0; i < NPETERSONLOCKS; i++) {
    petersonlocks[i].active = 0;
    petersonlocks[i].flag[0] = 0;
    petersonlocks[i].flag[1] = 0;
    petersonlocks[i].turn = 0;
  }
}

int
peterson_create(void)
{
  for(int i = 0; i < NPETERSONLOCKS; i++) {
    if(__sync_lock_test_and_set(&petersonlocks[i].active, 1) == 0) {
      petersonlocks[i].flag[0] = 0;
      petersonlocks[i].flag[1] = 0;
      petersonlocks[i].turn = 0;
      return i;
    }
  }
  return -1;
}

int
peterson_acquire(int lock_id, int role)
{
  if(lock_id < 0 || lock_id >= NPETERSONLOCKS || role < 0 || role > 1)
    return -1;
  struct petersonlock *lk = &petersonlocks[lock_id];
  if(!lk->active)
    return -1;

  __sync_lock_test_and_set(&lk->flag[role], 1);
  __sync_synchronize();
  lk->turn = 1 - role;
  __sync_synchronize();
  while(lk->flag[1 - role] && lk->turn == (1 - role)) {
    yield();
    __sync_synchronize();
  }
  return 0;
}

int
peterson_release(int lock_id, int role)
{
  if(lock_id < 0 || lock_id >= NPETERSONLOCKS || role < 0 || role > 1)
    return -1;
  struct petersonlock *lk = &petersonlocks[lock_id];
  if(!lk->active)
    return -1;

  __sync_lock_release(&lk->flag[role]);
  __sync_synchronize();
  return 0;
}

int
peterson_destroy(int lock_id)
{
  if(lock_id < 0 || lock_id >= NPETERSONLOCKS)
    return -1;
  struct petersonlock *lk = &petersonlocks[lock_id];
  if(!lk->active)
    return -1;
  lk->active = 0;
  return 0;
}