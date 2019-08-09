#include <stdio.h>

//#define MAX 3844
//#define MIN 37

void count_down( void ) {
    //  Insert your solution here
    for ( int i = 3844; i > 37; i -= 8 ) {
        printf("%d\n", i);
    }
}

int main( void ) {
	count_down( );
	return 0;
}