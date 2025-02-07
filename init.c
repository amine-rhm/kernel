#include <stdio.h>
#include "syscall.h"

int main() {
    int i=0;
	// i=spawn("./bar.so");
	// printf("./bar.so lance : %d\n",i);
	// i=spawn("./foo.so");
	// printf("./foo.so lance : %d\n",i);	
	// i=spawn("./foo.so");
	// printf("./foo.so lance 2: %d\n",i);
  i = spawn("./signal.so");
  printf("./signal.so lance : %d\n", i);

		// i = spawn("./send.so");
    // printf("./send.so lancé : %d\n", i);

	while (1) {
	      printf("init %d\n",mlk_clock());
	      // for (i=0;i<100000000;i++);
	      mlk_sleep(10);
	  }

}