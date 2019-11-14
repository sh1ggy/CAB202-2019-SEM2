#include <util/delay.h>
#include <cpu_speed.h>
#include <graphics.h>
#include <macros.h>

void setup(void) 
{
    set_clock_speed(CPU_8MHZ);
    lcd_init(LDC_DEFAULT_CONTRAST);
    clear_screen();
    show_screen();
}

void process(void) 
{
    // LED 0 is in register B, pin 2
    
    // PORT B is for using register B as an OUTPUT (PORTput)

    // PIN B is for using register B as an INPUT (PINput)

    // turn LED on

    // set bit 2 of PORT B to 1
    // 0b00000001 << 2 == 0b00000100
    PORTB = PORTB | (1 << 2);
}