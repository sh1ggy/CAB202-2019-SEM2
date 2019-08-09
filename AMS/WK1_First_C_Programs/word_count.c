#include <stdio.h>
#include <ctype.h>

void word_count() {
    int line_count = 0, char_count = 0,  word_count =0;
    int prev_c = 0;
    while(1) {
        char curr_c = getchar();
        // checking if it's at the end of the file 
        if ( curr_c == EOF ) {
            // checking the value of prev_c to break appropriately
            if ( prev_c == 0 ) {
                break;
            } else if ( prev_c == '\n' ) {
                break;
            } else {
                word_count++;
                break;
            }
        }
        //--- VERIFYING NEW WORD ---//
        // checking if the current character is not a space and not a newline
        // because everything other than a space or newline should be a word
        else if ( curr_c != ' ' && curr_c != '\n' ) {
            // checking to see if the code ran through the other verifiers to then verify that it is a word
            if ( prev_c == ' ' || prev_c == '\n' )  
                word_count++; // +1 WORD
            char_count++; // doing this so characters are still added in this condition
            prev_c = curr_c; // setting iteration condition to trigger else 
        }
        //--- VERIFYING NEW LINE ---//
        else if ( curr_c == '\n' ) {
            line_count++; // +1 LINE
            char_count++; // a new line is also considered a character
            prev_c = curr_c; // iteration condition = \n
        } 
        //--- VERIFYING NEW CHAR ---//
        else {
            char_count++;
            prev_c = curr_c;
        }
    } 

    printf("Document contents: %d lines; %d words; %d characters.\n", line_count, word_count, char_count);
}


int main() {
	word_count();
	return 0;
}
