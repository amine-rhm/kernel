#include <stdio.h>
#include <stdlib.h>

void main() {
  unsigned long long int value=0;
	printf("Debut main\n");
	while(value++<10000000000) if (rand()<RAND_MAX/100000000) printf("Hello world\n");
	printf("fini %d\n",value);
}
