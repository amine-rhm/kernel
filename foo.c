#include <stdio.h>
#include <stdlib.h>
#include "syscall.h"

void main() {
  int i=0,r;
  while (1) {
    printf("foo %d %d\n",mlk_clock(),mlk_getpid());
    for (i=0;i<100000;i++);
    r=rand()%5+1;
    mlk_sleep(r);
    }
}

