#include <stdio.h>
#include <syscall.h>
#include <stdlib.h>

int
main (int argc, char **argv){
	int fib = atoi(argv[1]);
	int argv1 = atoi(argv[1]);
	int argv2 = atoi(argv[2]);
	int argv3 = atoi(argv[3]);
	int argv4 = atoi(argv[4]);
	//printf("%d %d %d %d\n",argv1, argv2, argv3, argv4); 
	printf("%d %d\n", fibonacci(fib), max_of_four_int(argv1, argv2, argv3, argv4));
	return EXIT_SUCCESS;
}


