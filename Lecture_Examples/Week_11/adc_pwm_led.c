/*
**	adc_pwm.c
**
**	Demonstrates PWM, using Timer 4 to drive the complementary Output
**	Compare Register 4D, on pin D6. This is connected to the orange LED
**	mounted next to the reset button on the Teensy.
**
**	The PWM duty cycle is controlled by the value of the right
**	potentiometer to make the light dim or brighten.
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

//main loop
int main() {
	setup();

	for ( ;; ) {
		process();
		_delay_ms(50);
	}
}

//initialises ADC, PWM and USB port
void setup(void) {
	set_clock_speed(CPU_8MHz);

	// Enable orange LED
	SET_BIT(DDRD, 6);

	usb_init();
	while ( !usb_configured() ) {}

	adc_init();

	// Enable PWM on OC4D complement pin (D6). 
	//
	// (a)	Set the TOP value for the timer overflow comparator to 1023, 
	//		yielding 1024 ticks for timer 4 per overflow.
	TC4H = OVERFLOW_TOP >> 0x8;
	OCR4C = OVERFLOW_TOP & 0xff;

	// (b)	Not using OC4A or OC4B.
	TCCR4A = 0;

	// (c)	Set pre-scale.
	//		(Table 15-15, p165 of datasheet)

	// .... First, use a pre-scale of 1024 -- this is too slow for use to 
	//		experience dimming, but it clearly demonstrates the on-off behaviour.
	//		This will cause Timer counter 4 to update 8000000/1024 = 7812.5
	//		times per second, giving overflow frequency of 7812.5/1024 = 7.63
	//		cycles per second (approximately).
	// .... This is so slow we see it flickering
	// TCCR4B = BIT(PWM4X) | BIT(CS43) | BIT(CS41) | BIT(CS40);

	// .... Try pre-scale to 64, giving 8000000/64/1024 = 122.0703125Hz 
	// TCCR4B = BIT(CS42) | BIT(CS41) | BIT(CS40);

	// .... Try pre-scale to 128, giving 8000000/128/1024 = 61.03515625Hz 
	// TCCR4B = BIT(CS43);

	// .... Try pre-scale to 256, giving 8000000/256/1024 = 30.517578125Hz 
	TCCR4B = BIT(CS43) | BIT(CS40);

	// (d)	Enable complementary output on OC4D and turn on PWM.
	//		(Table 15-17, pp167)
	TCCR4C = BIT(COM4D0) | BIT(PWM4D);
	// DDRD |= BIT(6);

	//	(e)	Select FastPWM mode.
	//		(Table 15-19)
	TCCR4D = 0;
}

char buffer[80];

void set_duty_cycle(int duty_cycle) {
	// (a)	Set bits 8 and 9 of Output Compare Register 4D.
	TC4H = duty_cycle >> 8;

	// (b)	Set bits 0..7 of Output Compare Register 4D.
	OCR4D = duty_cycle & 0xff;
}

void process(void) {
	long right_adc = adc_read(1);

	// Set the duty cycle on Timer 4. Note that we have access to the 
	// _complement_ pin, so as the duty cycle increases, the time-averaged
	// voltage _decreases_.
	set_duty_cycle(OVERFLOW_TOP * (ADC_MAX - right_adc) / ADC_MAX);

	// Send debugging information via USB serial
	send_formatted( buffer, sizeof(buffer), "\rright_adc = %6d; PWM pin = %d \n", right_adc, BIT_IS_SET(PIND,6) );

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
