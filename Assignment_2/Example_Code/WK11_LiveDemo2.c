#include <stdint.h>
#include <avr/io.h> 
#include <avr/interrupt.h>
#include <util/delay.h>
#include <cpu_speed.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <macros.h>
#include <graphics.h>
#include <lcd_model.h>
#include "lcd.h"
#include "usb_serial.h"
#include "cab202_adc.h"


#define FREQ     (8000000.0)
#define PRESCALE0 (1.0) //  for a Freq of 7.8125Khz
#define PRESCALE1 (8.0)    //  for a Freq of 1Mhz
#define PRESCALE3 (1024.0)  //  for a Freq of 31.25Khz
#define DEBOUNCE_MS (70)  // ms delay for debouncing 

#define STATUS_BAR_HEIGHT 8

#define MAX_WALLS 4


//global declarations
uint8_t smiley_direct[8];
int XYw[MAX_WALLS][4];// = {x1,y1,x2,y2,.. }
double m1, m2,m3, m4;
int M1x, M1y;
double b1;
volatile int dx, dy;
int wall_length1 = 5;
int wall_length2 = 10;
int wall_length3 = 12;
int left_right_click, up_down_click;
volatile int duty_cycle=0;


//timing variables
volatile uint8_t overflow_counter0 = 0;
volatile uint32_t overflow_counter1 = 0;
volatile uint32_t overflow_counter3 = 0;
volatile double time_sec=0;
volatile int time_min=0;


//forward declarations
void draw_double(uint8_t x, uint8_t y, double value, colour_t colour);
void draw_int(uint8_t x, uint8_t y, int value, colour_t colour);
void draw_int16(uint8_t x, uint8_t y, uint16_t value, colour_t colour);
void setup_walls(void);
void draw_formatted(int x, int y, char * buffer, int buffer_size, const char * format, ...);
void usb_serial_send(char * message);



////////////  timer section ///////////////

//setup a 8 bit timer
void setup_timer0(void) {

		// Timer 0 in normal mode (WGM02), 
		// with pre-scaler 1024 ==> ~30Hz overflow 	(CS02,CS01,CS00).
		// Timer overflow on. (TOIE0)
		CLEAR_BIT(TCCR0B,WGM02);
		//prescaler 1
		
		CLEAR_BIT(TCCR0B,CS02);  //0
		CLEAR_BIT(TCCR0B,CS01); //0
		SET_BIT(TCCR0B,CS00);   //1
		
		//enabling the timer overflow interrupt
		SET_BIT(TIMSK0, TOIE0);

}

//setup a 16bit timer
void setup_timer1(void) {

		// Timer 1 in normal mode (WGM12. WGM13), 
		// with pre-scaler 8 ==> 	(CS12,CS11,CS10).
		// Timer overflow on. (TOIE1)

		CLEAR_BIT(TCCR1B,WGM12);
		CLEAR_BIT(TCCR1B,WGM13);
		CLEAR_BIT(TCCR1B,CS12);    //0 see table 14-6
		SET_BIT(TCCR1B,CS11);      //1
		CLEAR_BIT(TCCR1B,CS10);     //0
		
	    //enabling the timer overflow interrupt
		SET_BIT(TIMSK1, TOIE1);
		
}

//setup a 16bit timer
void setup_timer3(void) {

	   // Timer 3 in normal mode (WGM32. WGM33), 
		// with pre-scaler 256 ==> 	(CS32,CS31,CS30).
		// Timer overflow on. (TOIE3)

		CLEAR_BIT(TCCR3B,WGM32);
		CLEAR_BIT(TCCR3B,WGM33);
		SET_BIT(TCCR3B,CS32);     //1 see table 14-6
		CLEAR_BIT(TCCR3B,CS31);   //0
		SET_BIT(TCCR3B,CS30);   //0
		
		//enabling the timer overflow interrupt
		SET_BIT(TIMSK3, TOIE3);

}


//setup a 10bit timer
void setup_timer4(void){

}


ISR(TIMER0_OVF_vect) {	
	
	
	if (overflow_counter0 < duty_cycle){
	  SET_BIT(PORTB,3);
	  }
	else{
	  CLEAR_BIT(PORTB,3);
	  }	
	  
	overflow_counter0++;
	  	
	
}

ISR(TIMER1_OVF_vect) {
	overflow_counter1 ++;
	
    //increments
    // 16 * 65536 = 1048576 micro seconds approx 1.04 sec
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

////////////  end timer section ///////////////


int get_current_time(){
	
	time_sec = ( overflow_counter3 * 65535.0 + TCNT3 ) * PRESCALE3  / FREQ;
	
	if (time_sec >= 60.00) {
		overflow_counter3 = 0.0;
		time_min++;
        time_sec = 0.0;
	}

return (int)time_sec;
}



void draw_status_bar(){
	
  char buffer[80];

  draw_line(0,STATUS_BAR_HEIGHT,80,STATUS_BAR_HEIGHT,FG_COLOUR);
  int time = get_current_time();
  draw_formatted( 35, 0, buffer, sizeof(buffer), " T: %.2d:%.2d", time_min,time );

 
}




void check_buttons(){

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

//set up graphic objects to be displayed

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

void get_usb(void){

 if (usb_serial_available()){
 	char tx_buffer[32];
	int c = usb_serial_getchar(); //read usb port
	
	
	 if(c =='a'){ //
      left_right_click--;
      snprintf( tx_buffer, sizeof(tx_buffer), "received '%c'\r\n", c );
	  usb_serial_send( tx_buffer );
      
      }
    
     if(c =='d'){ //
      left_right_click++;
      snprintf( tx_buffer, sizeof(tx_buffer), "received '%c'\r\n", c );
	  usb_serial_send( tx_buffer );
      
      }
    
    }


}





void setup(void) {
    set_clock_speed(CPU_8MHz);
    lcd_init(LCD_DEFAULT_CONTRAST);
    clear_screen();
    setup_walls();
    setup_timer0();
    setup_timer1();
    setup_timer3();
    
    //enable global interrupts
    sei();
    
    //enable buttons
    CLEAR_BIT(DDRD, 1); // joystick up
	CLEAR_BIT(DDRB, 1); // joystick left
	CLEAR_BIT(DDRD, 0); // joystick right
	CLEAR_BIT(DDRB, 7);   //joystick down
	
	// Enable left LED for output
	SET_BIT(DDRB, 2);
	SET_BIT(DDRB, 3);
	
	//setup USB connection
	usb_init();
	
	//init ADC
	adc_init();

	while ( !usb_configured() ) {
		// Block until USB is ready.
	}
	
    
}



void process(void) {
	
	clear_screen();
    //debounce buttons
    check_buttons();
    //draw status bar
    draw_status_bar();
    //read usb port
    get_usb();
    //draw lines
    draw_walls();
    //draw a character
    draw_char(left_right_click,up_down_click, '@',FG_COLOUR);
    
    //duty cycle is adjusted with the potentiometer 
   int right_adc = adc_read(1);
   duty_cycle = (int)254.0 * (right_adc/1023.0);
 
   //add logic to cycle the variable duty_cycle from 0 - TOP - 0
   //that would gradually dim the LED
    
    
    
	//draw_formatted(15,24, buffer, sizeof(buffer), "%d", duty_cycle );
   
	show_screen();

    
}

int main(void) {
    setup();

    for (;;) {
        process();
        _delay_ms(3);
    }
    
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

void usb_serial_send(char * message) {
	// Cast to avoid "error: pointer targets in passing argument 1 
	//	of 'usb_serial_write' differ in signedness"
	usb_serial_write((uint8_t *) message, strlen(message));
}