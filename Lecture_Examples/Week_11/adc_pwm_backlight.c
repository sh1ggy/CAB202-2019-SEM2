/*
**	adc_pwm.c
**
**	Demonstrates PWM, using Timer 4 to drive the Output Compare Register 4A,
**	on pin C7. This is connected to the LCD backlight on the TeensyPewPew.
**
**	The PWM duty cycle is controlled by the value of the left
**	potentiometer to make the backlight dim or brighten.
**
**	Author: Lawrence Buckingham, Queensland University of Technology.
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
#include "usb_serial.h"
#include "cab202_adc.h"

void setup(void);
void process(void);
void draw_formatted(int x, int y, char * buffer, int buffer_size, const char * format, ...);
void send_formatted(char * buffer, int buffer_size, const char * format, ...);

#define BIT(x) (1 << (x))
#define OVERFLOW_TOP (1023)
#define ADC_MAX (1023)

char buffer[80];

int main() {
	setup();

	for ( ;; ) {
		process();
		_delay_ms(50);
	}
}


//initialise ADC, USB and PWM
void setup(void) {
	set_clock_speed(CPU_8MHz);

	usb_init();
	adc_init();
	lcd_init(LCD_DEFAULT_CONTRAST);

	// Enable PWM on OC4A (C7), which is wired to LCD backlight. 

	// (a)	Set the TOP value for the timer overflow comparator to 1023, 
	//		yielding a cycle of 1024 ticks per overflow.
	TC4H = OVERFLOW_TOP >> 8;
	OCR4C = OVERFLOW_TOP & 0xff;

	// (b)	Use OC4A for PWM. Remember to set DDR for C7.
	TCCR4A = BIT(COM4A1) | BIT(PWM4A);
	SET_BIT(DDRC, 7);

	// (c)	Set pre-scale to "no pre-scale" 
	TCCR4B = BIT(CS42) | BIT(CS41) | BIT(CS40);

	// (c.1) If you must adjust TCCR4C, be surgical. If you clear
	//		bits COM4A1 and COM4A0 you will _disable_ PWM on OC4A, 
	//		because the COM4Ax and COM4Bx hardware pins appear in
	//		both TCCR4A and TCCR4C!

	/* In this example, TCCR4C is not needed */

	// (d)	Select Fast PWM
	TCCR4D = 0;
}

void set_duty_cycle(int duty_cycle) {
	// (a)	Set bits 8 and 9 of Output Compare Register 4A.
	TC4H = duty_cycle >> 8;

	// (b)	Set bits 0..7 of Output Compare Register 4A.
	OCR4A = duty_cycle & 0xff;
}

void process(void) {
	long left_adc = adc_read(0);

	set_duty_cycle(ADC_MAX - left_adc);

	clear_screen();
	draw_string(0, 0, "ADC_PWM_BACKLIGHT", FG_COLOUR);
	draw_string(0, 10, "<-- Adjust pot", FG_COLOUR);
	draw_formatted(0, 20, buffer, sizeof(buffer), "ADC: %d", left_adc);

	show_screen();
}

void draw_formatted(int x, int y, char * buffer, int buffer_size, const char * format, ...) {
	va_list args;
	va_start(args, format);
	vsnprintf(buffer, buffer_size, format, args);
	draw_string(x, y, buffer, FG_COLOUR);
}

void send_formatted(char * buffer, int buffer_size, const char * format, ...) {
	va_list args;
	va_start(args, format);
	vsnprintf(buffer, buffer_size, format, args);
	usb_serial_write((uint8_t *) buffer, strlen(buffer));
}
