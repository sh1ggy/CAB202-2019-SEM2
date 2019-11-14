/*
**	adc_lcd.c
**
**	Demonstrates ADC control. 
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
#include "lcd_model.h"
#include "ascii_font.h"

void setup(void);
void process(void);

//main loop
int main() {
	setup();

	for ( ;; ) {
		process();
		_delay_ms(100);
	}
}


uint8_t smiley_original[8] = {
    0b00111100,
    0b01000010,
    0b10100101,
    0b10000001,
    0b10100101,
    0b10011001,
    0b01000010,
    0b00111100,
};

uint8_t smiley_direct[8];
uint8_t x, y;
char buffer[20];

/*
**  Convert smiley into vertical slices for direct drawing.
**  This will need to be amended if smiley is larger than 8x8.
*/
void setup_smiley(void) {
    // Visit each column of output bitmap
    for (int i = 0; i < 8; i++) {

        // Visit each row of output bitmap
        for (int j = 0; j < 8; j++) {
            // Kind of like: smiley_direct[i][j] = smiley_original[j][7-i].
            // Flip about the major diagonal.
            uint8_t bit_val = BIT_VALUE(smiley_original[j], (7 - i));
            WRITE_BIT(smiley_direct[i], j, bit_val);
        }
    }

    // Choose any random (x,y)
    x = rand() % (LCD_X - 8);
    y = rand() % (LCD_Y - 8);
}

/*
**  Draw smiley face directly to LCD.
**  (Notice: we cheat on the y-coordinate.)
*/
void draw_smiley(void) {
    LCD_CMD(lcd_set_function, lcd_instr_basic | lcd_addr_horizontal);
    LCD_CMD(lcd_set_x_addr, x);
    LCD_CMD(lcd_set_y_addr, y / 8);

    for (int i = 0; i < 8; i++) {
        LCD_DATA(smiley_direct[i]);
    }
}

/*
**	Remove smiley from LCD.
*/
void erase_smiley(void) {
    LCD_CMD(lcd_set_function, lcd_instr_basic | lcd_addr_horizontal);
    LCD_CMD(lcd_set_x_addr, x);
    LCD_CMD(lcd_set_y_addr, y / 8);

    for (int i = 0; i < 8; i++) {
        LCD_DATA(0);
    }
}


//initialise ADC and LCD
void setup(void) {
	set_clock_speed(CPU_8MHz);
	adc_init();
	lcd_init(LCD_DEFAULT_CONTRAST);
	lcd_clear();
	setup_smiley();
	draw_smiley();
}


void process(void) {

	//read right potentiometers
	int right_adc = adc_read(1);
	int left_adc = adc_read(0);	
	erase_smiley();
    double adc_x = (double) right_adc * (LCD_X - 8) / 1024;
    double adc_y = (double) left_adc * (LCD_Y - 8) / 1024;
	x = (int) adc_x;
	y = (int) adc_y;
	draw_smiley();
}
