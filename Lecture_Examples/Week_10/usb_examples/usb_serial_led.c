
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>

#include "usb_serial.h"

#include "lcd.h"
#include "graphics.h"
#include "cpu_speed.h"
#include "macros.h"

#define BUFF_LENGTH 64


//function declaration
void setup(void);
void setup_usb_serial(void);
void usb_serial_send(char * message);
void usb_serial_send_int(int value);



//initialise ports and LCD screen
void setup()
{

	//set clock speed to 8MhZ
    set_clock_speed(CPU_8MHz);
    
	SET_BIT(DDRB,2); 	//LED0 and LED1 as outputs
	SET_BIT(DDRB,3);
	
	CLEAR_BIT(DDRF,5); 	//Switch0 and Switch1 as input
	CLEAR_BIT(DDRF,6);
	
	PORTB = 0xF0; 	// Turn everything off to start with
	
	
	// Initialise the USB serial
    setup_usb_serial();
}


// Basic command interpreter for controlling port pins
int main(void)
{

    
    //initialise LCD and ports
	setup();
	
	clear_screen();
	//display a message on the screen
	draw_string(5,5, "USB serial demo",FG_COLOUR);
	show_screen();



   //keep looping until the character 'q' is received
	while (1) {
        
		// listen for commands and process them
	  

            if (usb_serial_available())
            {
            	// the buffer to store received characters
	
				char tx_buffer[BUFF_LENGTH];
	
            
			   int c = usb_serial_getchar(); //read usb port
			   //send a characters to the other side
			   if (c >= 0){
			    clear_screen();
			    snprintf( tx_buffer, sizeof(tx_buffer), "Teensy Received: %c \r\n", (char)c );
				usb_serial_send(tx_buffer);
				show_screen();
			   }
			   
			    //check character  and perform an action
            if(c =='a'){ //turn LED0 ON
                PORTB &= 0xF0;
                PORTB |=(1<<PINB2);
                }
            if(c =='d'){ //turn LED1 ON
            	PORTB &= 0xF0;
            	PORTB |= (1<<PINB3);
            	}
            if(c =='s'){ //turn both LEDs OFF
            	PORTB &= 0xF0;
				}
			
		              
		}
		
		     
		
	}	 
	return 0;
}


// ---------------------------------------------------------
//	USB serial business.
// ---------------------------------------------------------

void setup_usb_serial(void) {
	// Set up LCD and display message
	lcd_init(LCD_DEFAULT_CONTRAST);
	draw_string(10, 10, "Connect USB...", FG_COLOUR);
	show_screen();

	usb_init();

	while ( !usb_configured() ) {
		// Block until USB is ready.
	}

	clear_screen();
	draw_string(10, 10, "USB connected", FG_COLOUR);
	show_screen();

}

/*
**	Transmits a string via usb_serial.
*/
void usb_serial_send(char * message) {
	// Cast to avoid "error: pointer targets in passing argument 1 
	//	of 'usb_serial_write' differ in signedness"
	usb_serial_write((uint8_t *) message, strlen(message));
}

/*
**	Transmits an integer value via usb_serial.
*/

void usb_serial_send_int(int value) {
	static char buffer[8];
	snprintf(buffer, sizeof(buffer), "%d", value);
	usb_serial_send( buffer );
}

//-------------------------------------------------------------------

