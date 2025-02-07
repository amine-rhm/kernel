#include <stdio.h>
#include <stdlib.h>
#include "syscall.h"

void main() {
   static int wait_count = 0;
   while (1) {
          mlk_print("J'attends un signal");
          mlk_wait();
            printf("[WAIT] Signal reçu : %d\n", ++wait_count);
          mlk_sleep(2);
   }
}
