#include <stdint.h>
#include <avr/io.h> 
#include <avr/interrupt.h>
#include <util/delay.h>
#include <cpu_speed.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <macros.h>
#include <math.h>
#include <graphics.h>
#include <lcd_model.h>
#include "lcd.h"
#include "usb_serial.h"
#include "cab202_adc.h"

#define FREQ        (8000000.0)
#define PRESCALE0   (1024.0) //  for a Freq of 7.8125Khz
#define PRESCALE1   (8.0)    //  for a Freq of 1Mhz
#define PRESCALE3   (256.0)  //  for a Freq of 31.25Khz
#define DEBOUNCE_MS (100)     //  ms delay for debouncing 

#define STATUS_BAR_HEIGHT 8

#define MAX_WALLS 4
#define STEP_SIZE 1

//--- GLOBAL VARIABLES ---//

int game_over = 0, game_start = 1, game_paused = 0;
int game_level = 1, game_score = 0, seed;
bool level_2 = false;
int left_right_click, up_down_click;

//-- CHARACTER variables
int jerry_startx = 0, jerry_starty = STATUS_BAR_HEIGHT + 1;
double jerry_rx = 0, jerry_ry = STATUS_BAR_HEIGHT + 1; 

#define JERRY_BITS 14
#define JERRY_WIDTH 5
#define JERRY_HEIGHT 4
int jerry_lives = 5;
int jerry_x[14] = {0, 1, 3, 4, 0, 1, 3, 4, 1, 2, 3, 1, 2, 3};
int jerry_y[14] = {0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 3, 3, 3};

#define TOM_BITS 13
#define TOM_WIDTH 4
#define TOM_HEIGHT 4
int tom_startx = LCD_X - 5, tom_starty = LCD_Y - 9;
double tom_rx = LCD_X - 5, tom_ry = LCD_Y - 9, tom_dx, tom_dy; 
int tom_x[13] = {0, 4, 0, 1, 2, 3, 4, 0, 2, 4, 1, 2, 3};
int tom_y[13] = {0, 0, 1, 1, 1, 1, 1, 2, 2, 2, 3, 3, 3};

#define CHEESE_BITS 4 
#define CHEESE_WIDTH 2
#define CHEESE_HEIGHT 2
int num_cheese = 0, cheese_check = 0, cheese_flag = 2;
int cheese_x[4] = {0, 1, 0, 1};
int cheese_y[4] = {0, 0, 1, 1};

#define TRAP_BITS 4
#define TRAP_WIDTH 3
#define TRAP_HEIGHT 2
int num_traps = 0, trap_check, trap_flag = 3;
int trap_x[4] = {1, 0, 1, 2};
int trap_y[4] = {0, 1, 1, 1};

#define DOOR_BITS 11
#define DOOR_WIDTH 4
#define DOOR_HEIGHT 4
bool door_flag = false;
int door_rx, door_ry;
int door_x[11] = {1, 2, 0, 3, 0, 2, 3, 0, 1, 2, 3};
int door_y[11] = {0, 0, 1, 1, 2, 2, 2, 3, 3, 3, 3};

#define POT_BITS 5
#define POT_WIDTH 3
#define POT_HEIGHT 2
int pot_flag = 5;
int pot_x[5] = {0, 2, 0, 1, 2};
int pot_y[5] = {0, 0, 1, 1, 1};

#define SUPER_BITS 25
#define SUPER_WIDTH 7
#define SUPER_HEIGHT 5
int superjerry_rx, superjerry_ry, super_flag = 10;
bool superjerry = false;
int superjerry_x[25] = {1, 2, 4, 5, 0, 1, 2, 4, 5, 6, 0, 1, 2, 3, 4, 5, 6, 1, 2, 3, 4, 5, 2, 3, 4};
int superjerry_y[25] = {0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 3, 3, 3, 3, 3, 4, 4, 4};

struct item
{
	bool state;
	double rx, ry;
	double dx, dy;
};

struct item cheese[5];
struct item traps[5];
struct item pot;

// WEAPONS //
// double weap_coords[20][2];
// double weap_x, weap_y, weap_stepx, weap_stepy;
// int weap_count = 0;

//--- Timing variables
volatile int overflow_counter0 = 0;
volatile int cheese_counter = 0;
volatile int trap_counter = 0;
volatile uint32_t overflow_counter1 = 0;
volatile uint32_t overflow_counter3 = 0;
volatile int time_sec = 0;
volatile int cheese_sec = 0;
volatile int trap_sec = 0;
volatile int time_min = 0;

//--- Wall variables
volatile int wall_velocity;
volatile int wall_neg_velocity;
volatile int right_adc, left_adc;
int XYw[MAX_WALLS][4]; // = {x1,y1,x2,y2,.. }
volatile int dx, dy;
int x11,y11,x12,y12;
int x21,y21,x22,y22;
int x31,y31,x32,y32;
int x41,y41,x42,y42;

// Debouncing variables
volatile uint8_t switch_counter = 0;
volatile uint8_t switch_state;

//--- Forward declarations
void draw_double(uint8_t x, uint8_t y, double value, colour_t colour);
void draw_int(uint8_t x, uint8_t y, int value, colour_t colour);
void draw_int16(uint8_t x, uint8_t y, uint16_t value, colour_t colour);
void setup_walls(void);
void draw_formatted(int x, int y, char * buffer, int buffer_size, const char * format, ...);
void usb_serial_send(char * message);
void setup();
void setup_level_2();
void setup_cheese();
void setup_trap();
void setup_pot();
void end_game();
void setup_lvl2_walls();
void draw_walls();

//--- TIMERS ---//
//-- 8 bit timer
void setup_timer0(void) {
	// Timer 0 in normal mode (WGM02), 
	// with pre-scaler 1024 ==> ~30Hz overflow 	(CS02,CS01,CS00).
	// Timer overflow on. (TOIE0)
	CLEAR_BIT(TCCR0B,WGM02);
	SET_BIT(TCCR0B,CS02);  //1
	CLEAR_BIT(TCCR0B,CS01); //0
	SET_BIT(TCCR0B,CS00);   //1
	
	SET_BIT(TIMSK0, TOIE0); // enabling timer overflow interrupt
}

//-- 16bit timer
void setup_timer1(void) {
	// Timer 1 in normal mode (WGM12. WGM13), 
	// with pre-scaler 8 ==> 	(CS12,CS11,CS10).
	// Timer overflow on. (TOIE1)
	CLEAR_BIT(TCCR1B,WGM12);
	CLEAR_BIT(TCCR1B,WGM13);
	CLEAR_BIT(TCCR1B,CS12); //0 see table 14-6
	SET_BIT(TCCR1B,CS11);   //1
	CLEAR_BIT(TCCR1B,CS10); //0
	
	SET_BIT(TIMSK1, TOIE1); // enabling timer overflow interrupt
}

//-- 16bit timer
void setup_timer3(void) {
	// Timer 3 in normal mode (WGM32. WGM33), 
	// with pre-scaler 256 ==> 	(CS32,CS31,CS30).
	// Timer overflow on. (TOIE3)
	CLEAR_BIT(TCCR3B,WGM32);
	CLEAR_BIT(TCCR3B,WGM33);
	SET_BIT(TCCR3B,CS32);     //1 see table 14-6
	CLEAR_BIT(TCCR3B,CS31);   //0
	CLEAR_BIT(TCCR3B,CS30);   //0
	
	SET_BIT(TIMSK3, TOIE3); // enabling timer overflow interrupt
}

//-- 10bit timer
void setup_timer4(void) {
}

ISR(TIMER0_OVF_vect) {	
	if (game_paused == 0 && game_start == 0) {
		overflow_counter0++;
		cheese_sec++;
		seed++;
	}

	if (superjerry) {
		SET_BIT(PORTB, 3);
		SET_BIT(PORTB, 2);
	}
	else {
		CLEAR_BIT(PORTB, 3);
		CLEAR_BIT(PORTB, 2);
	}

	// DEBOUNCING //
	switch_counter = switch_counter<<1;

    uint8_t mask = 0b00111111;
    switch_counter = switch_counter & mask;

    switch_counter = switch_counter | BIT_IS_SET(PINF, 6); // left button
	switch_counter = switch_counter | BIT_IS_SET(PINF, 5); // right button
	switch_counter = switch_counter | BIT_IS_SET(PIND, 1); // joystick up
	switch_counter = switch_counter | BIT_IS_SET(PINB, 1); // joystick left
	switch_counter = switch_counter | BIT_IS_SET(PIND, 6); // joystick right
	switch_counter = switch_counter | BIT_IS_SET(PINB, 6); // joystick down

    if (switch_counter == mask) {
        switch_state = 1;
    }
    if (switch_counter == 0) {
        switch_state = 0;
    }
	// END DEBOUNCING // 
}

ISR(TIMER1_OVF_vect) {
	if (game_paused == 0) {
		overflow_counter1++;
	}
	
	int middle_potent = 511;

	if (right_adc >= middle_potent) {
		if (overflow_counter1 > wall_velocity) {
			dx = (dx + 1) ;
			dy = (dy + 1) ;
			overflow_counter1 = 0;
		}
	}
	if (right_adc <= (middle_potent - 1) ) {
		if (overflow_counter1 > wall_neg_velocity) {
			dx = (dx - 1) ;
			dy = (dy - 1) ;
			overflow_counter1 = 0;
		}
	}
    
}

ISR(TIMER3_OVF_vect) {
	overflow_counter3++;
}

//--- END TIMERS ---//

int get_current_time() {
	time_sec = ( overflow_counter0 * 256.0 + TCNT0 ) * PRESCALE0  / FREQ;
	
	if (time_sec >= 60.00) {
		overflow_counter0 = 0.0;
		time_min++;
        time_sec = 0.0;
	}
	return (int)time_sec;
}

// [FUNCTION] Creates a new cheese, setting all the appropriate setup variables
void new_cheese()
{
    for (int i = 0; i < 5; i++)
    {
        if (!cheese[i].state)
        {
			cheese[i].rx = rand() % LCD_X - 5;
			cheese[i].ry = rand() % (LCD_Y - STATUS_BAR_HEIGHT + 2) + STATUS_BAR_HEIGHT - 2;
            cheese[i].state = true;
            num_cheese++;
            return;
        }
    }
}

// [FUNCTION] Creates a new trap, setting all the appropriate setup variables
void new_trap()
{
    for (int i = 0; i < 5; i++)
    {
        if (!traps[i].state)
        {
			traps[i].rx = tom_rx;
			traps[i].ry = tom_ry;
			traps[i].state = true;
            num_traps++;
            return;
        }
    }
}

// [FUNCTION] Creates a new pot, setting all the appropriate setup variables
void new_pot()
{
	if (!pot.state)
	{
		pot.rx = tom_rx;
		pot.ry = tom_ry;
		pot.state = true;
		return;
	}
}

// [TIME] Timer for cheese
int get_cheese_time() {
	cheese_sec = ( cheese_counter * 256.0 + TCNT0 ) * PRESCALE0 / FREQ;
	return(int)cheese_sec;
	// set cheese_counter to zero when a cheese is collected
}

// [TIME] Timer for traps
int get_trap_time() {
	trap_sec = ( trap_counter * 256.0 + TCNT0 ) * PRESCALE0 / FREQ;
	return(int)trap_sec;
}

// [TIME] Checks to see if it's an appropriate time to spawn a cheese
void check_cheese_time() {
	int time = get_current_time();
	if (time == cheese_flag) {
		if (num_cheese < 5) {
			new_cheese();
		}
		cheese_flag = time + 2;
	}
}

// [TIME] Checks to see if it's an appropriate time to spawn a trap
void check_trap_time() {
	int time = get_current_time();
	if (time == trap_flag) {
		if (num_traps < 5) {
			new_trap();
		}
		trap_flag = time + 3;
	}
}

void check_pot_time() {
	int time = get_current_time();
	if (time == pot_flag && level_2) {
		new_pot();
		pot_flag = time + 5;
	}
}

void check_super_time() {
	int time = get_current_time();
	if (time == super_flag && superjerry == true) {
		superjerry = false;
		super_flag = time + 10;
	}	
}

// [FUNCTION] Shows a starting game screen
void start_game() {
	while ( game_start == 1 ) {
		clear_screen();
		draw_string(0, 10, "Tyrone Nolasco", FG_COLOUR);
		draw_string(0, 20, "n10232117", FG_COLOUR);
		draw_string(0, 30, "Tom and Jerry", FG_COLOUR);
		if ( BIT_IS_SET(PINF, 5) ) {
			game_start = 0;
			game_paused = 0;
			_delay_ms(DEBOUNCE_MS);
		}
		show_screen();
	}
}

void setup_level_2() {
	level_2 = true;
	game_paused = 0;

	cheese_check = 0, trap_check = 0, num_cheese = 0, num_traps = 0;
	cheese_flag = get_current_time() + 2;
	trap_flag = get_current_time() + 3;
	pot_flag = get_current_time() + 5;
	cheese_counter = 0, trap_counter = 0;
	dy = 0, dx = 0;
	setup_cheese();
	setup_trap();
	setup_pot();
	setup_lvl2_walls();

	superjerry = false;
	door_flag = false;

	jerry_rx = tom_startx;
	jerry_ry = tom_starty;
	tom_rx = jerry_startx + 5;
	tom_ry = jerry_starty + 5;
	// setup();
}

// [FUNCTION] Restarts all elements to initial properties
void restart_setup() {
	jerry_rx = jerry_startx, jerry_ry = jerry_starty, jerry_lives = 5, game_score = 0;
	tom_rx = tom_startx, tom_ry = tom_starty;
	time_min = 0, time_sec = 0, overflow_counter0 = 0;
	game_paused = 0;
	dy = 0, dx = 0;

	cheese_check = 0, trap_check = 0, num_cheese = 0, num_traps = 0;
	cheese_flag = 2, trap_flag = 3, pot_flag = 5;
	cheese_counter = 0, trap_counter = 0;
    setup_cheese();
    setup_trap();
	setup_pot();
	setup_walls();
	door_flag = false;
	superjerry = false;
	level_2 = false;
	game_level = 1;
}

void next_level() {
	if ( BIT_IS_SET(PINF, 6) && !level_2) {
		_delay_ms(DEBOUNCE_MS + 100);
		setup_level_2();
		game_level++;
	}
	else if ( BIT_IS_SET(PINF, 6) && level_2 ) {
		_delay_ms(DEBOUNCE_MS + 100);
		game_over = 1;
		end_game();
	}
	
}

// [FUNCTION] Shows a game over screen 
void end_game() {
	while ( game_over == 1 ) {
		clear_screen();
		draw_string(0, 10, "GAME OVER", FG_COLOUR);
		draw_string(0, 20, "Right switch", FG_COLOUR);
		draw_string(0, 30, "to start over :(", FG_COLOUR);
		if ( BIT_IS_SET(PINF, 5) ) {
			restart_setup();
			_delay_ms(DEBOUNCE_MS);
			game_over = 0;
			game_paused = 0;
		}
		show_screen();
	}
}

// [FUNCTION] Toggles pause mode if the right switch is closed
void pause_game() {
	if ( BIT_IS_SET(PINF, 5) && game_paused == 0 && game_start != 1 && game_over != 1) {
		_delay_ms(DEBOUNCE_MS + 100);
		game_paused = 1;
	}
	else if ( BIT_IS_SET(PINF, 5) ) {
		_delay_ms(DEBOUNCE_MS + 100);
		game_paused = 0;
	}
}

// [DRAW] UI: draws the status bar
void draw_status_bar() {
	char buffer[80];
	int time = get_current_time();
	draw_formatted(0, 0, buffer, sizeof(buffer), "L%.d HP%.d S%.2d %.2d:%.2d", game_level, jerry_lives, game_score, time_min, time );
	draw_line(0,STATUS_BAR_HEIGHT, LCD_X, STATUS_BAR_HEIGHT, FG_COLOUR);
}

void clear_status_bar() {
	for (int x = 0; x < 84; x++)
	{
		screen_buffer[x] = 0;
	}
}

// [FUNCTION] Draws the bitmap specified
void draw_bitmap(int num_bits, int refx, int refy, int arrayx[], int arrayy[]) {
	for (int i = 0; i < num_bits; i++) {
		draw_pixel(refx + arrayx[i], refy + arrayy[i], FG_COLOUR);
	}
}

void setup_walls() {
	// WALL 1
	XYw[0][0] = 18;
	XYw[0][1] = 15;
	XYw[0][2] = 13;
	XYw[0][3] = 25;		 

	// WALL  2
	XYw[1][0] = 25;
	XYw[1][1] = 35;
	XYw[1][2] = 25;
	XYw[1][3] = 45;
	
	// WALL 3
	XYw[2][0] = 45;
	XYw[2][1] = 10;
	XYw[2][2] = 60;
	XYw[2][3] = 10;
	
	// WALL 4
	XYw[3][0] = 58;
	XYw[3][1] = 25;
	XYw[3][2] = 72;
	XYw[3][3] = 30;
}

void setup_lvl2_walls() {
	XYw[0][0] = 76;
	XYw[0][1] = 70;
	XYw[0][2] = 73;
	XYw[0][3] = 80;

	XYw[1][0] = 58;
	XYw[1][1] = 26;
	XYw[1][2] = 58;
	XYw[1][3] = 36;

	XYw[2][0] = 12;
	XYw[2][1] = 32;
	XYw[2][2] = 27;
	XYw[2][3] = 32;

	XYw[3][0] = 46;
	XYw[3][1] = 13;
	XYw[3][2] = 60;
	XYw[3][3] = 18;
}


void do_walls()
{
    draw_line(x11,y11,x12,y12,FG_COLOUR);
    draw_line(x21,y21,x22,y22,FG_COLOUR);
   	draw_line(x31,y31,x32,y32,FG_COLOUR);
  	draw_line(x41,y41,x42,y42,FG_COLOUR);
	
	clear_status_bar();
}


void draw_walls(void){

		//First line
	x11 = (XYw[0][0] - dx) ; 		
	y11 = (XYw[0][1] - dy) ;
	x12 = (XYw[0][2] - dx) ;		 
	y12 = (XYw[0][3] - dy) ;

   	if (y11 > LCD_Y)
   	{
	   	XYw[0][0] = XYw[0][0] - LCD_Y + STATUS_BAR_HEIGHT;
	   	XYw[0][1] = XYw[0][1] - LCD_Y + STATUS_BAR_HEIGHT;
		XYw[0][2] = XYw[0][2] - LCD_Y + STATUS_BAR_HEIGHT;
	   	XYw[0][3] = XYw[0][3] - LCD_Y + STATUS_BAR_HEIGHT;
   	}

	if (y12 < STATUS_BAR_HEIGHT + 1)
	{
		XYw[0][0] = XYw[0][0]  + LCD_Y - STATUS_BAR_HEIGHT + 1;
	   	XYw[0][1] = XYw[0][1]  + LCD_Y - STATUS_BAR_HEIGHT + 1;
	   	XYw[0][2] = XYw[0][2]  + LCD_Y - STATUS_BAR_HEIGHT + 1;
	   	XYw[0][3] = XYw[0][3]  + LCD_Y - STATUS_BAR_HEIGHT + 1;				   
   	}
    
    //second line

    x21 = (XYw[1][0] + dx);
	if (x21 < 0)
	{
		XYw[1][0] = XYw[1][0] + LCD_X;	
	}
	if (x21 > LCD_X)
	{
		XYw[1][0] = XYw[1][0] - LCD_X - 1;
	}

    y21 = XYw[1][1];

    x22 = (XYw[1][2]) + dx;

	if (x22 < 0)
	{
		XYw[1][2] = XYw[1][2] + LCD_X;
	}
	if (x22 > LCD_X)
	{
		XYw[1][2] = XYw[1][2] - LCD_X - 1;
	}

    y22 = XYw[1][3];
    // //third line
	x31 = XYw[2][0];

    y31 = (XYw[2][1] + dy) ;
	if (y31 < STATUS_BAR_HEIGHT + 1)
	{
		XYw[2][1] = XYw[2][1] + LCD_Y - STATUS_BAR_HEIGHT - 1;
	}
	if (y31 > LCD_Y)
	{
		XYw[2][1] = XYw[2][1] - LCD_Y + STATUS_BAR_HEIGHT;
	}

    x32 = XYw[2][2];

    y32 = (XYw[2][3] + dy) ;
	if (y32 < STATUS_BAR_HEIGHT + 1)
	{
		XYw[2][3] = XYw[2][3] + LCD_Y - STATUS_BAR_HEIGHT - 1;
	}
	if (y32 > LCD_Y)
	{
		XYw[2][3] = XYw[2][3] - LCD_Y + STATUS_BAR_HEIGHT;
	}
    //fourth line
    x41 = (XYw[3][0] + dx);
    y41 = (XYw[3][1] - dy);
    x42 = (XYw[3][2] + dx);
    y42 = (XYw[3][3] - dy);
	if (y41 > LCD_Y)
   	{
	   	XYw[3][0] = XYw[3][0] + LCD_Y + STATUS_BAR_HEIGHT - 16;
	   	XYw[3][1] = XYw[3][1] - LCD_Y + STATUS_BAR_HEIGHT;
		XYw[3][2] = XYw[3][2] + LCD_Y + STATUS_BAR_HEIGHT - 16;
	   	XYw[3][3] = XYw[3][3] - LCD_Y + STATUS_BAR_HEIGHT;
   	}
	
	if (y42 < STATUS_BAR_HEIGHT + 1)
	{
		XYw[3][0] = XYw[3][0] - LCD_Y - STATUS_BAR_HEIGHT + 16;
	   	XYw[3][1] = XYw[3][1] + LCD_Y - STATUS_BAR_HEIGHT;
	   	XYw[3][2] = XYw[3][2] - LCD_Y - STATUS_BAR_HEIGHT + 16;
	   	XYw[3][3] = XYw[3][3] + LCD_Y - STATUS_BAR_HEIGHT;		
	}
    
    
	do_walls();
}

// [DRAW] TOM: draws Tom according to the bitmap
void draw_tom() {
	draw_bitmap(TOM_BITS, tom_rx, tom_ry, tom_x, tom_y);
}

// [DRAW] JERRY: draws Jerry according to the bitmap
void draw_jerry() {
	draw_bitmap(JERRY_BITS, jerry_rx, jerry_ry, jerry_x, jerry_y);
}

void draw_superjerry() {
	draw_bitmap(SUPER_BITS, superjerry_rx, superjerry_ry, superjerry_x, superjerry_y);
}

void setup_superjerry() {
	superjerry_rx = jerry_rx;
	superjerry_ry = jerry_ry;
}

// [SETUP] CHEESE: initialises all the states of cheeses to false
void setup_cheese()
{
    for (int i = 0; i < 5; i++)
    {
        cheese[i].state = false;
    }
}

// [SETUP] TRAP: initialises all the states of traps to false
void setup_trap()
{
    for (int i = 0; i < 5; i++)
    {
        traps[i].state = false;
    }
}

// [SETUP] POTION: initialises state of potion
void setup_pot() {
	pot.state = false;
}

// [DRAW] DOOR: draws door according to the bitmap
void draw_door() {
	if (door_flag) {
		draw_bitmap(DOOR_BITS, door_ry, door_rx, door_x, door_y);
	}
}

// [SETUP] DOOR: initialises the reference point of the door to random coordinates
void setup_door() {
	door_rx = 30;
	door_ry = 30;
	door_flag = false;
}

// [DRAW] CHEESE: draws all cheeses if needed according to the bitmap
void draw_cheese() {
    for (int i = 0; i < 5; i++) {
        if (cheese[i].state == true) {
			draw_bitmap(CHEESE_BITS, cheese[i].rx, cheese[i].ry, cheese_x, cheese_y);
        }
    }
}

// [DRAW] TRAP: draws all traps if needed according to the bitmap
void draw_trap() {
    for (int i = 0; i < 5; i++) {
        if (traps[i].state == true) {
			draw_bitmap(TRAP_BITS, traps[i].rx, traps[i].ry, trap_x, trap_y);
        }
    }
}

void draw_pot() {
	if (pot.state) {
		draw_bitmap(POT_BITS, pot.rx, pot.ry, pot_x, pot_y);
	}
}

// [DO] CHEESE: collision checks for cheese 
void do_cheese() {
	for (int i = 0; i < 5; i++) { // For all the cheeses
		/// Checks if the cheese is in the play field
		if (cheese[i].state) {
			// Checks box collisions around the cheese with Jerry
			if ( (round(jerry_rx) < round(cheese[i].rx + CHEESE_WIDTH)) 
			&& (round(jerry_rx + JERRY_WIDTH) > round(cheese[i].rx)) ) {
				if ( (round(jerry_ry) < round(cheese[i].ry + CHEESE_HEIGHT)) 
				&& (round(jerry_ry + JERRY_HEIGHT) > round(cheese[i].ry)) ) {
					game_score++; // Increasing score

					// Cheese setup after collision to reset
					num_cheese--;
					cheese_check++;
					cheese[i].rx = 5 + rand() % LCD_X - 5;
					cheese[i].ry = 8 + rand() % (LCD_Y - STATUS_BAR_HEIGHT + 3) + STATUS_BAR_HEIGHT;
					cheese[i].state = false;
					cheese_flag = get_current_time() + 2;
					if (cheese_check == 5) {
						door_flag = true;
					}
				}
			}
		}

	}
}

// [DO] TRAP: collision checks for mousetraps  
void do_trap() {
	for (int i = 0; i < 5; i++) { // For all the cheese
		// Checks if the trap is in the play field
		if (traps[i].state) {
			// Checks box collisions around the trap with Jerry 
			if ( (round(jerry_rx) < round(traps[i].rx + TRAP_WIDTH)) 
			&& (round(jerry_rx + JERRY_WIDTH) > round(traps[i].rx)) ) {
				if ( (round(jerry_ry) < round(traps[i].ry + TRAP_HEIGHT)) 
				&& (round(jerry_ry + JERRY_HEIGHT) > round(traps[i].ry)) ) {
					if (!superjerry) {
						jerry_lives--; // Subtract lives
					}
					else if (superjerry) {
						game_score++;
					}
					// Game over check
					if (jerry_lives == 0) {
						game_over = 1;
						end_game();
					}
					// Trap setup after collision to reset
					num_traps--;
					traps[i].rx = tom_rx;
					traps[i].ry = tom_ry;
					traps[i].state = false;
					trap_flag = get_current_time() + 2;
				}
			}
		}
	}
}

void do_pot() {
	if (pot.state) {
		if ( (round(jerry_rx) < round(pot.rx + POT_WIDTH)) 
		&& (round(jerry_rx + JERRY_WIDTH) > round(pot.rx)) ) {
			if ( (round(jerry_ry) < round(pot.ry + POT_HEIGHT)) 
			&& (round(jerry_ry + JERRY_HEIGHT) > round(pot.ry)) ) {
				pot.state = false;
				superjerry = true;
				setup_superjerry();
				pot_flag = get_current_time() + 5;
				super_flag = get_current_time() + 10;
			}
		}
	}
}

// [DO] DOOR: Checks if Jerry has collided with the door
void do_door() {
	if (door_flag) {
		if ( (round(jerry_rx) < round(door_rx + DOOR_WIDTH)) 
		&& (round(jerry_rx + JERRY_WIDTH) > round(door_rx)) ) {
			if ( (round(jerry_ry) < round(door_ry + DOOR_HEIGHT)) 
			&& (round(jerry_ry + JERRY_HEIGHT) > round(door_ry)) ) {
				if (!level_2) {
					setup_level_2();
					game_score++;
				}
				else if (level_2) {
					game_over = 1;
					end_game();
				}
			}
		}
	}
	
} 

// [DRAW] Draws all the appropriate draw functions
void draw_all() {
	clear_screen();
	if (!superjerry) {
		draw_jerry();
	}
	else if (superjerry) {
		draw_superjerry();
	}
	draw_tom();
	draw_cheese();
	draw_trap();
	draw_walls();
	draw_status_bar();
	draw_door();
	draw_pot();
	show_screen();
}

// void do_weap() {
// 	if ( BIT_IS_SET(PINB, 0) ) {
// 		init_weap();
// 		double p1 = ((tom_rx + TOM_WIDTH) / 2) -  ((jerry_rx + JERRY_WIDTH) /2);
// 		double p2 = ((tom_ry + TOM_HEIGHT) / 2) -  ((jerry_ry + JERRY_HEIGHT) /2);
// 		double distance = sqrt(p1*p1 + p2*p2);
// 		weap_stepx = p1 * 0.2 / distance;
// 		weap_stepy = p2 * 0.2 / distance;
// 		weap_count++;

// 	}
// 	// int new_x = weap_x + weap_stepx;
// 	// int new_y = weap_y + weap_stepy;

// 	if ()
// 	weap_coords[weap_count][0] += weap_stepx;
// 	weap_coords[weap_count][1] += weap_stepy;
// 	weap_count++;
// }

void do_jerry() {
	// Detect a Click on left button
	if ( BIT_IS_SET(PINB, 1) && jerry_rx > 0) {
		jerry_rx -= STEP_SIZE;
		superjerry_rx -= STEP_SIZE;
	}

	// Detect a click on right button
	if ( BIT_IS_SET(PIND, 0) && jerry_rx < LCD_X - 5) {
		jerry_rx += STEP_SIZE;
		superjerry_rx += STEP_SIZE;
	}

	// Detect a click on up joystick
	if ( BIT_IS_SET(PIND, 1) && jerry_ry > STATUS_BAR_HEIGHT + 1) {
		jerry_ry -= STEP_SIZE;	
		superjerry_ry -= STEP_SIZE;	
	}
	
	// Detect a click on down joystick
	if ( BIT_IS_SET(PINB, 7) && jerry_ry < LCD_Y - 4 ) {
		jerry_ry += STEP_SIZE;
		superjerry_ry += STEP_SIZE;
	}	
	// if (switch_state == 1) {

	// }
}

void setup_tom() {
	double tom_dir = rand() * 3.14159 * 2 / RAND_MAX;

	tom_dx = ( STEP_SIZE * cos(tom_dir) );
	tom_dy = ( STEP_SIZE * sin(tom_dir) );
}

void move_tom() {
	int new_x = round(tom_rx + tom_dx);
	int new_y = round(tom_ry + tom_dy);

	int bounced = 0;

	if (new_x == 0 || new_x == LCD_X - 4) {
		tom_dx = -tom_dx;
		bounced = 1;
	}
	if (new_y == STATUS_BAR_HEIGHT + 1 || new_y == LCD_Y - 3) {
		tom_dy = -tom_dy;
		bounced = 1;
	}

	if (bounced == 0) {
		tom_rx += tom_dx;
		tom_ry += tom_dy;
	}
}

void do_tom() {
	// Collision with Tom and Jerry
	if ( (round(jerry_rx) < round(tom_rx + TOM_WIDTH)) && (round(jerry_rx + JERRY_WIDTH) > round(tom_rx)) ) {
		if ( (round(jerry_ry) < round(tom_ry + TOM_HEIGHT)) & (round(jerry_ry + JERRY_HEIGHT) > round(tom_ry)) ) {
			// Resetting Tom and Jerry's positions
			tom_rx = tom_startx, tom_ry = tom_starty;
			if (!superjerry) {
				jerry_rx = jerry_startx, jerry_ry = jerry_starty;
			}
			if (!superjerry) {
				jerry_lives--; // Deducting lives
			}
			else if (superjerry) {
				game_score++;
			}
			
			if (jerry_lives == 0) {
				game_over = 1;
				end_game();
			}
		}
	}
}

void get_usb(void){
	if (usb_serial_available()) {
		int c = usb_serial_getchar(); //read usb port

        if (c =='w' && jerry_ry > STATUS_BAR_HEIGHT + 1 ) { 
            jerry_ry -= STEP_SIZE;
        }

        if (c =='a' && jerry_rx > 0) { //
            jerry_rx -= STEP_SIZE;
        }

        if (c =='s'&& jerry_ry < LCD_Y - 4 ) { //
            jerry_ry += STEP_SIZE;
        }

        if (c =='d' && jerry_rx < LCD_X - 5) { //
            jerry_rx += STEP_SIZE;
        }
    }
}

// [FUNCTION] WALLS: changing the speed 
void wall_speed() {
	right_adc = adc_read(1);
	left_adc = adc_read(0);
	double middle_potent = 511;
	double max_potent = 1023;
	double min_potent = 0;
	double new_max = 1;
	double new_min = 10;
	wall_velocity = ( ( (right_adc - middle_potent) * (new_max - new_min)) / (max_potent - middle_potent) ) + new_min;
	wall_neg_velocity = ( ( (right_adc - min_potent) * (new_min - new_max)) / ((middle_potent-1) - min_potent) ) + new_max;
}


void setup() {
	clear_screen();
    set_clock_speed(CPU_8MHz);
    lcd_init(LCD_DEFAULT_CONTRAST);

		
	//setup USB connection
	usb_init();

	while ( !usb_configured() ) {
		// Block until USB is ready.
	}

    setup_walls();
	setup_tom();
	setup_cheese();
	setup_trap();
	setup_door();
	// setup_weap();

    setup_timer0();
    setup_timer1();
    setup_timer3();

    sei(); //enable global interrupts
    
	CLEAR_BIT(DDRF, 6); // left button
	CLEAR_BIT(DDRF, 5); // right button

    CLEAR_BIT(DDRD, 1); // joystick up
	CLEAR_BIT(DDRB, 1); // joystick left
	CLEAR_BIT(DDRD, 0); // joystick right
	CLEAR_BIT(DDRB, 7); // joystick down
	CLEAR_BIT(DDRB, 0); // joystick centre
	
	SET_BIT(DDRB, 2); // Enable left LED for output
	SET_BIT(DDRB, 3); // Enable right LED for output

	adc_init();


	// Run introduction 
	if (!level_2) {
		game_start = 1;
		start_game();
	}
	
}

void check_door() {
	if (cheese_check > 4) {
		door_flag = true;
	}
}

void process(void) {
	draw_all();
	srand(seed);
	
	pause_game();

	check_door();
	do_door();
	do_jerry();
	do_tom();
	do_cheese();
	do_trap();
	do_pot();
	// do_weap();

	if (game_paused == 0) {
		check_cheese_time();
		check_trap_time();
		check_pot_time();
		check_super_time();
		move_tom();
		// move_walls();
	}
    
    // Read usb port
    get_usb();

	next_level();

	wall_speed();
}

int main(void) {
    setup();
    for (;;) {
        process();
        _delay_ms(50);
    }
    return 0;
}

// -------------------------------------------------
//    Helper functions.
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


void usb_serial_send(char * message) {
	// Cast to avoid "error: pointer targets in passing argument 1 
	//	of 'usb_serial_write' differ in signedness"
	usb_serial_write((uint8_t *) message, strlen(message));
}
