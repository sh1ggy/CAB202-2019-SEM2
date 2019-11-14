#include <stdio.h>
#include <stdbool.h>

int main() {
    bool done = false;
    const double fact = 1.609344;

    while ( ! done ) {
        printf( "Please enter a command:\nm == miles to kilos\nk == kilos to miles\nx == exit\n" );

        int ch = getchar();

        if ( ch == 'm') {
            double miles;
            printf("Please enter miles:\n");
            scanf("%lf", &miles);
            double ks = fact * miles;
            printf( "%f miles is %f ks\n", miles, ks );
        }
        else if ( (ch == 'x') || (ch == EOF) ) {
            done = true;
        }
    }
}