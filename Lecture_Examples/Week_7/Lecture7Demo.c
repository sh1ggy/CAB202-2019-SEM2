
#include <avr/io.h> 
#include "cpu_speed.h" // 8 MHz (encouraged) or 16MHz
#include "macros.h"

#include "graphics.c"

// screen coordinates 
int dx = 4;
int dy = 2;

int dx_l = 10;
int dy_l = 10;

void setup() 
{
    set_clock_speed(CPU_8MHZ);

    // initialise the screen
    lcd_init(LCD_DEFAULT_CONTRAST);
    
    // set port B pin 2 as output, put 1 in location 2 of register DDRB
    SET_BIT(DDRB, 2);

    // set port F pin 6 as input, put 0 in location 6 of register DDRF
    CLEAR_BIT(DDRF, 6); 

    // set port F pin 5 as input, put 0 in location 5 of register DDRF
    CLEAR_BIT(DDRF, 5);

    clear_screen();
    
    draw_string(dx, dy, "Moving String", FG_COLOUR);
    draw_line(dx_l, dy_l, dx_l+10, dy_l+20, FG_COLOUR); // can't have same x / y coords

    show_screen();

}

void process() 
{
    // if button is pressed then turn LED ON
    if ( BIT_IS_SET(PINF, 6) ) // check state of pin 6 of port F
    {
        SET_BIT(PORTB, 2); // turning LED on

        dx++;
        dy++;

        dx_l++;
        dy_l++;


    } 
    else if ( BIT_IS_SET(PINF, 5) ) 
    {
        CLEAR_BIT(PORTB, 2); // turning LED off

        dx--;
        dy--;

        dx_l--;
        dy_l--;
    }
}

void main() 
{
    setup();
    for(;;) 
    {
        
    }
    process();
}