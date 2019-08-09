#include <stdio.h>
#include <stdlib.h>

//  Declare an array called rolled_vals with capacity for 85 elements
//  of the default signed integer type.
int rolled_vals[85];

void roll_dice( void ) {
    //  Seed the random number generator with the value 750476.
    srand(750476); // makes it more random 

    //  Generate 85 random values between 1 and 18 inclusive. To do 
    //  this, use a counter-controlled loop which will visit every element of 
    //  rolled_vals. The counter should start at zero, and the loop should continue 
    //  while the counter is less than 85.
    
    for ( int i = 0; i < 86 ; i++ ) {
        //  Generate a random value between 1 and 18 inclusive. Store this 
        //  value in the current element of rolled_vals - that is, the element 
        //  indexed by the loop counter.
        rolled_vals[i] = rand() % 18 + 1; // rand(); is in reverse
    } 
    //  End the loop.
}

int main(void) {
    roll_dice();

    // Display contents of array rolled_vals.
    for (int i = 0; i < 85; i++) {
        if (i > 0) {
            printf(",");
        }
        printf("%d", rolled_vals[i]);
    }

    printf("\n");

    return 0;
}
