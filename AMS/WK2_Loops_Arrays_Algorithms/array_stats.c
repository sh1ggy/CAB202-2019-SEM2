#include <stdio.h>
#include <stdlib.h>
#include <float.h>

//  Declare Global variables here.
double term[35], term_mean, term_min, term_max, term_sum;
int term_count;

void array_stats() {
    //  Insert your solution here.
    term_count = 0;
    term_max = 0;
    term_min = 0;
    term_mean = 0;
    term_sum = 0;
    
    // looping through the text document and counting each of the terms
    while( term_count < 35 && scanf("%lf", &term[term_count]) == 1 ) {
        term_count++;
    }

    // checking if number of terms in the list is not zero
    if (term_count != 0) {
        // summing all terms for mean calculation
        for ( int i = 0; i < term_count; i++ ) {
            term_sum += term[i];
        }

        term_mean = term_sum/term_count; // calculating final mean

        term_min = 100000000000; // making term_min really big so that the check passes
        // looping through all terms to find the lowest number
        for (int j = 0; j < term_count; j++) {
            if ( term[j] < term_min ) {
                term_min = term[j];
            }
        }

        // looping through all the terms to find the highest number
        for (int k = 0; k < term_count; k++) {
            if ( term[k] > term_max ) {
                term_max = term[k];
            }
        }
    }


}

#include <stdlib.h>
#include <time.h>

int main() {
    // Simulate the test setup process.
    srand( time( NULL ) );
    for ( int i = 0; i < 35; i++ ) {
        term[i] = rand();
    }
    term_count = rand();
    term_mean = rand();
    term_min = rand();
    term_max = rand();

    // Call submitted code.
    array_stats();

    // Display contents of array term.
    for (int i = 0; i < term_count; i++) {
        printf("%f ", term[i]);
    }

    printf("\n");
    printf("Item count: %d\n", term_count);
    printf("Item mean : %f\n", term_mean);
    printf("Item min  : %f\n", term_min);
    printf("Item max  : %f\n", term_max);

    return 0;
}