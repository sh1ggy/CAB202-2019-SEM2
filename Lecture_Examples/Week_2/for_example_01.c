#include <stdio.h>

#define MIN 1
#define MAX 101

int main () {
	printf("Printing integers from %d to %d.\n", MIN, (MAX-1));

	for ( int i = MIN; i < MAX; i++ ) {
		printf("%d\n", i);
	}

	printf("\nBye\n");
	return 0;
}
