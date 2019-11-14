#include <stdio.h>

#define MIN 1
#define MAX 101
#define STEP 7

int main () {
	printf("Printing integers from %d to %d, incrementing by %d.\n", 
		MIN, (MAX-1), STEP);

	for ( int i = MIN; i < MAX; i += STEP ) {
		printf("%d\n", i);
	}

	printf("\nBye\n");
	return 0;
}
