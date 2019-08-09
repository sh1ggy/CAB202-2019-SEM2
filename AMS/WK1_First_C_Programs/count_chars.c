#include <stdio.h>

void count_chars() {
    //  Declare an integer variable which will be used to count characters.
    //  Initialise the variable to zero.
    int count = 0;
    //  Enter a loop which continues indefinitely.
    while(count > -1) {
        // Read a single character code from standard input.
        char c = getchar();
        // If the character code is equal to EOF, break out of the loop.
        if (c == EOF) {
            break;
        }
        else {
            // Increment the counter by 1.
            count++;
        }
    } // End the body of the loop.

    // Display the number of characters processed.
    printf("The document contains %d characters.\n", count);
}


int main() {
	count_chars();
	return 0;
}
