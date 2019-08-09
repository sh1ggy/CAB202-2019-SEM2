#include <stdio.h>

void echo_char() {
    //  Prompt the user to enter a single character. 
    //  Use the text "Please enter a single character" followed by a single
    //  colon, and nothing else (either visible or invisible).
    //  The message should appear on a line by itself.
    //  Hint: printf.
    char c;

    //  Fetch a character code from standard input. 
    //  Hint: fgetc, getc, or getchar.
    printf( "Please enter a single character:\n" );
    c = getchar();

    printf( "You entered the character '" );
    putchar(c);
    printf( "'.\n" );
}