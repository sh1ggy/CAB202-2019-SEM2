/*
**	adc_led.c
**
**	Demonstrates ADC control. Each potentiometer is
**	assigned to a LED. When the ADC value is below a threshold
**	of 50% max (512) then the LED is off; when it is above the
**	threshold the LED is on.
**
**	This program is derived from an earlier unattributed source file.
**
**	Current custodian: Lawrence Buckingham, Queensland University of
**	Technology.
*/

#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "lcd.h"
#include "cpu_speed.h"
#include "graphics.h"
#include "macros.h"
#include "cab202_adc.h"

void setup(void);
void process(void);
void draw_formatted(int x, int y, char * buffer, int buffer_size, const char * format, ...);

#define THRESHOLD 512

int main() {
	setup();

	for ( ;; ) {
		process();
	}
}

//initialise ADC and LCD, and LDEDs
void setup(void) {
	set_clock_speed(CPU_8MHz);

	// Enable LEDS 0 and 1
	SET_BIT(DDRB, 2);
	SET_BIT(DDRB, 3);

	adc_init();
	lcd_init( LCD_DEFAULT_CONTRAST );
}

char buffer[80];

void process(void) {

	//read both potentiometers
	int left_adc = adc_read(0);
	int right_adc = adc_read(1);
	
	//turn LED on/off based on adc value
	WRITE_BIT(PORTB, 2, left_adc >= THRESHOLD);
	WRITE_BIT(PORTB, 3, right_adc >= THRESHOLD);

	clear_screen();
	draw_string(0, 0, "ADC_LED", FG_COLOUR);
	draw_formatted( 0, 10, buffer, sizeof(buffer), " Left ADC: %d", left_adc );
	draw_formatted( 0, 20, buffer, sizeof(buffer), "Right ADC: %d", right_adc );
	show_screen();
}

void draw_formatted(int x, int y, char * buffer, int buffer_size, const char * format, ...) {
	va_list args;
	va_start(args, format);
	vsnprintf(buffer, buffer_size, format, args);
	draw_string(x, y, buffer, FG_COLOUR);
}
