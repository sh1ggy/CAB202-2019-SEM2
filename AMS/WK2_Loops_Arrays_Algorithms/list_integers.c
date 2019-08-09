#include <stdio.h>
// may be better practice to declare constant upper and lower bounds
//#define MIN 68
//#define MAX 339

void list_integers( void ) {
    // Display the title message.
    printf("The integer sub-range from 68 to 338...\n");
    //  Enter a counter-controlled loop which will start at 68 and
    //  continues up to and including 338, advancing by increments of 1. 
    for ( int i = 68; i < 339; i++ ) {
        //  Print the current value of the counter variable.
        printf("%d\n", i);
    }
    // End the loop.
}