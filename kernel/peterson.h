#ifndef _PETERSON_H_
#define _PETERSON_H_

#include "param.h"

#define NPETERSONLOCKS 15

struct petersonlock {
  int active;           // 1 if this lock is in use, 0 otherwise
  volatile int flag[2]; // Peterson's flag for each process
  volatile int turn;    // Peterson's turn variable
};

void petersonlocks_init(void);

#endif