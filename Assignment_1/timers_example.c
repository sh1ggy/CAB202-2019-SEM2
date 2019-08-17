#include <stdio.h>
#include <stdlib.h>
#include "cab202_graphics.h"
#include "cab202_timers.h" // timer_id is a new variable type

timer_id timer1;
int seconds;

void setup() {
    timer1 = create_timer(1000); // in ms
}

int main() {
    setup_screen(); // allows drawing to screen
    setup();
    int key = get_char(); 
    while (key != 'q') {
        // basically counting the seconds according to the 1000ms timer
        if(timer_expired(timer1)) {
            seconds++;
        } // from this you could also develop a minutes variable that counts the amount of seconds passsed
        draw_int(10,10,seconds);
        show_screen();
        key = get_char();
    }
    cleanup_screen();
    return 0;
}