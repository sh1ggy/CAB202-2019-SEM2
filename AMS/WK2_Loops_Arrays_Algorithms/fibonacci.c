#include <stdio.h>
#include <stdlib.h>

//  Declare an array called term with capacity for 33 elements
//  of the default signed integer type.
int term[33];

void fibonacci( void ) {
    //  Insert your solution here.
    scanf("%d",&term[1]); // you can't have a variable = to scanf();
    scanf("%d",&term[2]);

    for ( int i = 2; i < 34; i++  ) {
        term[i] = term[i-1] + term[i-2]; 
    }

}

int main(void) {
    fibonacci();

    // Display contents of array term.
    for (int i = 0; i < 33; i++) {
        printf("%d\n", term[i]);
    }

    return 0;
}
