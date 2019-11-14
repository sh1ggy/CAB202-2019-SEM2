/*
**	usb_serial_hello.c
**
**	Demonstrates serial transmission from Teensy to desktop via USB.
**
**	Lawrence Buckingham, QUT, October 2017.
**	(C) Queensland University of Technology.
*/
#include <stdint.h>
#include <string.h>
#include <avr/io.h> 
#include <avr/interrupt.h>
#include <util/delay.h>
#include <cpu_speed.h>
#include <macros.h>
#include <graphics.h>

#include "usb_serial.h"

void setup_usb_serial( void );
void usb_serial_send(char * message);

void setup(void) {
	set_clock_speed(CPU_8MHz);

	// Enable centre LED for output
	SET_BIT(DDRD, 6);

	// Set Timer 0 to overflow approx 122 times per second.
	TCCR0B |= 4;
	TIMSK0 = 1;

	// Enable interrupts.
	sei();

	setup_usb_serial();
}

void process(void) {
	char *hello = "Hello USB!!!!";
	usb_serial_send( hello );
}

int main(void) {
	setup();

	for ( ;; ) {
		process();
		_delay_ms(2000);
	}
}

// ---------------------------------------------------------
//	Timer overflow business.
// ---------------------------------------------------------

#define FREQ 8000000.0
#define PRESCALE 256.0
#define TIMER_SCALE 256.0

double interval = 0;

ISR(TIMER0_OVF_vect) {
	interval += TIMER_SCALE * PRESCALE / FREQ;

	if ( interval >= 1.0 ) {
		interval = 0;
		PORTD ^= 1 << 6;
	}
}


// ---------------------------------------------------------
//	USB serial business.
// ---------------------------------------------------------

void usb_serial_send(char * message) {
	// Cast to avoid "error: pointer targets in passing argument 1 
	//	of 'usb_serial_write' differ in signedness"
	usb_serial_write((uint8_t *) message, strlen(message));
}

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
