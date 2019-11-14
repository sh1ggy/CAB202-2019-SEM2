/*
**	uart_hello.c
**
**	Demonstrates bidirectional communication between two TeensyPewPew
**	over UART.
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

#include "uart.h"

#define BAUD 9600

// ----------------------------------------------------------

void setup(void) {
	set_clock_speed(CPU_8MHz);
	lcd_init(LCD_DEFAULT_CONTRAST);
	
	uart_init(BAUD);

	// Enable centre LED for output
	SET_BIT(DDRD, 6);

	// Set Timer 0 to overflow approximately 122 times per second.
	TCCR0B |= 4;
	TIMSK0 = 1;

	// Enable interrupts.
	sei();
}

// ----------------------------------------------------------

void process(void) {
	clear_screen();
	draw_string(0, 0, "UART Demo", FG_COLOUR);

	static char sent_char = 'a';

	uart_putchar(sent_char);

	draw_string(0, 10, "TX:", FG_COLOUR);
	draw_char(3 * 5, 10, sent_char, FG_COLOUR);

	//increment alphabet
	sent_char++;
	//reset and start from 'a' again
	if ( sent_char > 'z' ) sent_char = 'a';

	if ( uart_available() ) {
		char received_char = uart_getchar();
		draw_string(0, 20, "RX:", FG_COLOUR);
		draw_char(3*5, 20, received_char, FG_COLOUR);
	}

	show_screen();
}

// ----------------------------------------------------------

int main(void) {
	setup();

	for ( ;; ) {
		process();
		_delay_ms(1000);
	}
}

// ---------------------------------------------------------
//	Timer overflow business.
// ---------------------------------------------------------

#define FREQ 8000000.0
#define PRESCALE 256.0
#define TIMER_SCALE 256.0

// Proof of life: toggle LED at 0.5Hz
ISR(TIMER0_OVF_vect) {
	static volatile double interval = 0;

	// Add timer overflow period to interval
	interval += TIMER_SCALE * PRESCALE / FREQ;

	if ( interval >= 0.5 ) {
		interval = 0;
		PORTD ^= 1 << 6;

		// Equivalent:
		// uint8_t current_bit = BIT_VALUE(PORTD, 6);
		// WRITE_BIT(PORTD, 6, (1 - current_bit));
	}
}

