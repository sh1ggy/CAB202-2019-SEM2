#include <stdio.h>

#define MIN 85
#define MAX 3676

void cab_202( void ) {
    // Insert your solution here.

    for ( int i = MIN; i < MAX; i++ ) {
        int cab = i%5;
        int c_202 = i%19;
        if ( cab == 0 && c_202 == 0 ) {
            printf("CAB202\n");
        } else if ( c_202 == 0 ) {
            printf("202\n");
        } else if ( cab == 0 ) {
            printf("CAB\n");
        } else {
            printf("%d\n", i);
        }
    }
}

int main( void ) {
	cab_202( );
	return 0;
}