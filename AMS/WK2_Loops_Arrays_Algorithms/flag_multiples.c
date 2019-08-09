#include <stdio.h>

//#define MIN 85
//#define MAX 3676

void flag_multiples( void ) {
    //  Enter a counter controlled loop which starts at 85 and
    //  advances by increments of 1 up to and including 3675
    for ( int i = 85; i < 3676; i++ ) {
        //  Send the current value of the counter to standard output as a 
        //  decimal formatted integer.
        printf("%d", i);
        int flag = i%5;

        //  If the current value of the counter is divisible by 5 then 
        //  highlight the current line by sending the pattern " <===" to 
        //  standard output.
        if ( flag == 0 ) {
            printf(" <===\n");
        } 
        else
        {
            printf("\n");
        }
        

        
    }


    //  End loop.
}

int main( void ) {
	flag_multiples( );
	return 0;
}