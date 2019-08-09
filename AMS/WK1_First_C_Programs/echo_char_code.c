#include <stdio.h>

void echo_char_code() {
    //  Insert your solution here.
    char c;

    printf( "Please enter a char value:\n" );
    c = getchar();

    printf( "The character code of '" );
    // Based on the character code from getchar(); put 
    // the appropriate character. 
    putchar(c);
    // Using format specifiers, interpolate 
    // - similar to C#
    printf( "' is %d.\n", c);
}


int main() {
	echo_char_code();
	return 0;
}