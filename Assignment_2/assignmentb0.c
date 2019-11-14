#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <cpu_speed.h>
#include <graphics.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <lcd.h>
#include <macros.h>
#include "lcd_model.h"
#include <math.h>

// Global variables
#define FREQ     (8000000.0)
#define PRESCALE0 (1024.0) //  for a Freq of 7.8125Khz
#define PRESCALE1 (8.0)    //  for a Freq of 1Mhz
#define PRESCALE3 (256.0)  //  for a Freq of 31.25Khz
#define DEBOUNCE_MS (70)  // ms delay for debouncing 

#define STATUS_BAR_HEIGHT 8
#define MAX_WALLS 4

int game_over = 0, game_start = 1, game_paused = 1, command = 0, status = 0;
int left_right_click, up_down_click;

int XYw[MAX_WALLS][4];// = {x1,y1,x2,y2,.. }
volatile int dx, dy;

//timing variables
volatile uint8_t overflow_counter0 = 0;
volatile uint32_t overflow_counter1 = 0;
volatile uint32_t overflow_counter3 = 0;
volatile double time_sec = 0;
volatile int time_min = 0;

//forward declarations
void draw_double(uint8_t x, uint8_t y, double value, colour_t colour);
void draw_int(uint8_t x, uint8_t y, int value, colour_t colour);
void draw_int16(uint8_t x, uint8_t y, uint16_t value, colour_t colour);
void setup_walls(void);
void draw_formatted(int x, int y, char * buffer, int buffer_size, const char * format, ...);
// void usb_serial_send(char * message);

double J_RX = 0, J_RY = STATUS_BAR_HEIGHT + 1; // starting coords
int jerry_bits = 14;
// 0b11011000,
// 0b11011000,
// 0b01110000,
// 0b01110000
double J_X[14] = {0, 1, 3, 4, 0, 1, 3, 4, 1, 2, 3, 1, 2, 3};
double J_Y[14] = {0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 3, 3, 3};


//--- TIMERS ---// 

//setup a 8 bit timer
void setup_timer0(void) {
		// Timer 0 in normal mode (WGM02), 
		// with pre-scaler 1024 ==> ~30Hz overflow 	(CS02,CS01,CS00).
		// Timer overflow on. (TOIE0)
		CLEAR_BIT(TCCR0B,WGM02);
		//prescaler 1
		CLEAR_BIT(TCCR0B,CS02); //0
		CLEAR_BIT(TCCR0B,CS01); //0
		SET_BIT(TCCR0B,CS00);   //1
		SET_BIT(TIMSK0, TOIE0); //enabling timer overflow interrupt
}

//setup a 16bit timer
void setup_timer1(void) {
		// Timer 1 in normal mode (WGM12. WGM13), 
		// with pre-scaler 8 ==> 	(CS12,CS11,CS10).
		// Timer overflow on. (TOIE1)
		CLEAR_BIT(TCCR1B,WGM12);
		CLEAR_BIT(TCCR1B,WGM13);
		CLEAR_BIT(TCCR1B,CS12); //0 see table 14-6
		SET_BIT(TCCR1B,CS11);   //1
		CLEAR_BIT(TCCR1B,CS10); //0
		
		SET_BIT(TIMSK1, TOIE1); //enabling timer overflow interrupt
	
}

//setup a 16bit timer
void setup_timer3(void) {
	   // Timer 3 in normal mode (WGM32. WGM33), 
		// with pre-scaler 256 ==> 	(CS32,CS31,CS30).
		// Timer overflow on. (TOIE3)

		CLEAR_BIT(TCCR3B,WGM32);
		CLEAR_BIT(TCCR3B,WGM33);
		SET_BIT(TCCR3B,CS32);   //1 see table 14-6
		CLEAR_BIT(TCCR3B,CS31); //0
		SET_BIT(TCCR3B,CS30);   //0
		
		SET_BIT(TIMSK3, TOIE3); //enabling timer overflow interrupt
 
}

//setup a 10bit timer
void setup_timer4(void){
}

ISR(TIMER0_OVF_vect) {	
	overflow_counter0 ++;
}

ISR(TIMER1_OVF_vect) {
	overflow_counter1 ++;
    //increments; 16*65536 = 1048576 micro seconds approx 1.04 sec
    // walls update and LED toggle at 1Hz
    if (overflow_counter1 > 10){
		dx = (dx + 1) ;//% (1/1); 
		dy = (dy + 1) ;//% (1/1); 
	    overflow_counter1=0;
	    //PORTB^=(1<<2);
	   }
}

ISR(TIMER3_OVF_vect) {
	overflow_counter3 ++;
}

void check_buttons() {
	// Detect a Click on left button
	if ( BIT_IS_SET(PINB, 1) ) {
		_delay_ms(DEBOUNCE_MS);
		while ( BIT_IS_SET(PINB, 1) ) {
			// Block until button released.
		}
		// Button has now been pressed and released...
		left_right_click = left_right_click - 2;
	}
	
	// Detect a Click on right button
	if ( BIT_IS_SET(PIND, 0) ) {
		_delay_ms(DEBOUNCE_MS);
		while ( BIT_IS_SET(PIND, 0) ) {
			// Block until button released.
		}
		// Button has now been pressed and released...
		left_right_click = left_right_click + 2;
	}

	// Display and wait if joystick up.
	if ( BIT_IS_SET(PIND, 1) ) {
		_delay_ms(DEBOUNCE_MS);
		while ( BIT_IS_SET(PIND, 1) ) {
			// Block until joystick released.
		}
		// Button has now been pressed and released...
		up_down_click = up_down_click - 2;	
	}
	
	// Display and wait if joystick down.
	if ( BIT_IS_SET(PINB, 7) ) {
		_delay_ms(DEBOUNCE_MS);
		while ( BIT_IS_SET(PINB, 7) ) {
			// Block until joystick released.
		}
		// Button has now been pressed and released...
		up_down_click = up_down_click + 2;	
	}	
}

// Current time
int get_current_time(){
	time_sec = ( overflow_counter0 * 256.0 + TCNT0 ) * PRESCALE0 / FREQ;
	if (time_sec >= 60.00) {
		overflow_counter0 = 0.0;
		time_min++;
        time_sec = 0.0;
	}
	return (int)time_sec;
}

// Drawing status bar
void draw_status() {
	char buffer[80];
	int time = get_current_time();
	draw_formatted( 35, 0, buffer, sizeof(buffer), " T: %.2d:%.2d", time_min,time );
	draw_line( 0, STATUS_BAR_HEIGHT, 83, STATUS_BAR_HEIGHT, FG_COLOUR );
}

void draw_walls(void){
   
    int x11,y11,x12,y12;
   int x21,y21,x22,y22;
    int x31,y31,x32,y32;
   int x41,y41,x42,y42; 
    
    //First line
    x11 = ( XYw[0][0] + dx ) ; if (x11 < 0 || x11 > LCD_X){dx=0;dy=0;}
    y11 = ( XYw[0][1] + dy ) ; if (y11 < 0 || y11 > LCD_Y){dy=0;dx=0;}
    x12 = ( XYw[0][2] + dx ) ; if (x12 < 0 || x12 > LCD_X){dx=0;dy=0;} 
    y12 = ( XYw[0][3] + dy );
    
    //second line
    x21 = ( XYw[1][0]-dx ) % LCD_X;
    y21 = XYw[1][1];
    x22 = ( XYw[1][2]-dx ) % LCD_X;
    y22 = XYw[1][3];
    //third line
    x31 = XYw[2][0];
    y31 = (XYw[2][1]-dy ) % LCD_Y;
    x32 = XYw[2][2];
    y32 = (XYw[2][3]-dy) % LCD_Y;
    //fourth line
    x41 = (XYw[3][0]+dx) % LCD_X;
    y41 = XYw[3][1];
    x42 = (XYw[3][2]+dx) % LCD_X;
    y42 = XYw[3][3];
    
    
	//draw 4 lines
    draw_line(x11,y11,x12,y12,FG_COLOUR);
    draw_line(x21,y21,x22,y22,FG_COLOUR);
   	draw_line(x31,y31,x32,y32,FG_COLOUR);
   	draw_line(x41,y41,x42,y42,FG_COLOUR);
}

void draw_jerry() {
	for (int i = 0; i < jerry_bits; i++) {
		draw_pixel(J_RX + J_X[i], J_RY + J_Y[i], FG_COLOUR);
	}
}

void draw_all() {
	clear_screen();
	draw_status();
	draw_jerry();
	show_screen();
}

void start_game() {
	while ( game_start == 1 ) {
		clear_screen();
		draw_string(0, 10, "Tyrone Nolasco", FG_COLOUR);
		draw_string(0, 20, "n10232117", FG_COLOUR);
		draw_string(0, 30, "Tom and Jerry", FG_COLOUR);
		if ( BIT_IS_SET(PINF, 6) ) {
			game_start = 0;
		}
		show_screen();
	}
}

void setup_walls(){
		XYw[0][0] = 18;
		XYw[0][1] = 15;
		XYw[0][2] = 13;
		XYw[0][3] = 25;		 
		//
		XYw[1][0] = 25;
		XYw[1][1] = 35;
		XYw[1][2] = 25;
		XYw[1][3] = 45;
		//
		XYw[2][0] = 45;
		XYw[2][1] = 10;
		XYw[2][2] = 60;
		XYw[2][3] = 10;
		//
		XYw[3][0] = 58;
		XYw[3][1] = 25;
		XYw[3][2] = 72;
		XYw[3][3] = 30;
}


void setup(void) {
	// Initialising the LCD //
	set_clock_speed(CPU_8MHz);
	lcd_init(LCD_DEFAULT_CONTRAST);
    
	setup_walls();
	setup_timer0();
    setup_timer1();
    setup_timer3();
    
    //enable global interrupts
    sei();

	// Readying joystick for input 
	CLEAR_BIT(DDRB, 1); // left 
	CLEAR_BIT(DDRD, 0); // right 
	CLEAR_BIT(DDRD, 1); // up 
	CLEAR_BIT(DDRB, 7); // down 
	CLEAR_BIT(DDRB, 0); // middle click 

	// Readying buttons for input //
	CLEAR_BIT(DDRF, 6); // left 
    CLEAR_BIT(DDRF, 5); // right 

	// Run introduction 
	game_start = 1;
	start_game();	
}



void process(void) {
    check_buttons();
}

int main(void) {
	// Run setup 
	setup();
	
	// Drawing to the buffer and sending to the LCD //
	draw_all();
	
	// Looping updates to the game
	while ( game_over == 0 ) {
		process();
		_delay_ms(100);
	}
	// End the script //
	return 0;
}

// -------------------------------------------------
// Helper functions.
// -------------------------------------------------


void draw_double(uint8_t x, uint8_t y, double value, colour_t colour) {
	char buffer[20];
	snprintf(buffer, sizeof(buffer), "%f", value);
	draw_string(x, y, buffer, colour);
}


void draw_int(uint8_t x, uint8_t y, int value, colour_t colour) {
	char buffer[20];
	snprintf(buffer, sizeof(buffer), "%d", value);
	draw_string(x, y, buffer, colour);
}

void draw_int16(uint8_t x, uint8_t y, uint16_t value, colour_t colour) {
	char buffer[32];
	snprintf(buffer, sizeof(buffer), "%u", (unsigned int)value);
	draw_string(x, y, buffer, colour);
}

void draw_formatted(int x, int y, char * buffer, int buffer_size, const char * format, ...) {
	va_list args;
	va_start(args, format);
	vsnprintf(buffer, buffer_size, format, args);
	draw_string(x, y, buffer, FG_COLOUR);
}

/*
void usb_serial_send(char * message) {
	// Cast to avoid "error: pointer targets in passing argument 1 
	//	of 'usb_serial_write' differ in signedness"
	usb_serial_write((uint8_t *) message, strlen(message));
}
*/