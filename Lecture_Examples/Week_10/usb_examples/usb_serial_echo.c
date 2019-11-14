/*
**	usb_serial_echo.c
**
**	Demonstrates character receive and transmit over USB serial.
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
#include <stdio.h>
#include "usb_serial.h"

void setup( void );
void process( void );
void setup_usb_serial( void );
void usb_serial_send(char * message);

// ----------------------------------------------------------

int main(void) {
	setup();

	for ( ;; ) {
		process();
	}
}

// ----------------------------------------------------------

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

// ----------------------------------------------------------

char buffer[81];

void process(void) {

	int16_t char_code = usb_serial_getchar();

	if ( char_code >= 0 ) {
		snprintf( buffer, sizeof(buffer), "received '%c'\r\n", char_code );
		usb_serial_send( buffer );
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

	if ( interval >= 0.1 ) {
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
