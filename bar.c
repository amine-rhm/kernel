#include <stdio.h>
#include "syscall.h"

void main() {
  unsigned long int i=0;
  while (1) {
    printf("bar %d\n",mlk_clock());
//    for (i=0;i<10000000000;i++);
    mlk_sleep(1);

  }
}

