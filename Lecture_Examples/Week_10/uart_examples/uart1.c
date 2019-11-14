
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>

#include "lcd.h"
#include "graphics.h"
#include "cpu_speed.h"
#include "macros.h"


#include "uart.h"


#define BUFF_LENGTH 20
// define some macros
#define BAUD 9600        // define baud rate




//initialise ports and LCD screen
void setup()
{

	
	//set clock speed to 8MhZ
    set_clock_speed(CPU_8MHz);
    
	SET_BIT(DDRB,2); 	//LED0 and LED1 as outputs
	SET_BIT(DDRB,3);
	
	CLEAR_BIT(DDRF,5);
	CLEAR_BIT(DDRF,6);	//Switch0 and Switch1 as input
	
	PORTB = 0xF0; 	// Turn everything off to start with
	lcd_init( LCD_DEFAULT_CONTRAST );
	clear_screen();
	show_screen();
	
	// Initialise the USB serial
    uart_init(BAUD);
}



// Basic command interpreter for controlling port pins
int main(void)
{

	
	// the buffer to store received characters
	char rcv;
  
    
    //initialise LCD and ports
	setup();
	
	clear_screen();
	//display a message on the screen
	draw_string(5,5, "UART serial demo",FG_COLOUR);
	show_screen();
	
	static char sent_char_a = 'a';
	static char sent_char_b = 'b';

	
		// main loop
	   while (1) {
			 
			 //detect button pressed
			 if (BIT_IS_SET(PINF,5)){
			 
			        clear_screen();
  					draw_string(0,10, "Button 3 pressed",FG_COLOUR);
  					draw_string(0,20, "Sending 'a' ",FG_COLOUR);
  					uart_putchar(sent_char_a); 
  					show_screen();
  				}
				
			     //detect button pressed
			     if (BIT_IS_SET(PINF,6)){
			        clear_screen();
  					draw_string(0,10, "Button 2 pressed",FG_COLOUR);
  					draw_string(0,20, "Sending 'b' ",FG_COLOUR);
  					uart_putchar(sent_char_b); 
  					show_screen();
  				}   
               
               	if ( uart_available() ) {
               	//receive character
               		rcv = uart_getchar();
               		//turn LED on depending on the character
               		if (rcv =='c'){ SET_BIT(PORTB, 3); CLEAR_BIT(PORTB,2);}
               		else if (rcv =='d'){ SET_BIT(PORTB, 2); CLEAR_BIT(PORTB,3);}
               
              		//draw received character
              		draw_string(0,30, "rx:",FG_COLOUR);
              		draw_char(0,38, rcv,FG_COLOUR);
              		show_screen();
              }
              
           
              
           _delay_ms(10);       
		}
		
		     
		
	 
	return 0;
}




