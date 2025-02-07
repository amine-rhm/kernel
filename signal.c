#include <stdio.h>
#include <stdlib.h>
#include "syscall.h"

void main() {
   int p;

   static int signal_count = 0;
   p = spawn("./wait.so");
   while (1) {
          mlk_print("J'envoie un signal");
          printf("[SIGNAL] Nombre de signaux envoyés : %d\n", ++signal_count);
          mlk_signal(p);
          mlk_sleep(1);
   }
}
