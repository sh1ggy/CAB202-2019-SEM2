#include <stdint.h>
#include <avr/io.h> 
#include <avr/interrupt.h>
#include <util/delay.h>
#include <cpu_speed.h>
#include <stdio.h>
#include <stdlib.h>

#include <macros.h>
#include <lcd_model.h>
#include <ascii_font.h>
#include <graphics.h>

/*
**  Initialise the LCD display.
*/
void new_lcd_init(uint8_t contrast) {
    // Set up the pins connected to the LCD as outputs
    SET_OUTPUT(DDRD, SCEPIN); // Chip select -- when low, tells LCD we're sending data
    SET_OUTPUT(DDRB, RSTPIN); // Chip Reset
    SET_OUTPUT(DDRB, DCPIN);  // Data / Command selector
    SET_OUTPUT(DDRB, DINPIN); // Data input to LCD
    SET_OUTPUT(DDRF, SCKPIN); // Clock input to LCD

    CLEAR_BIT(PORTB, RSTPIN); // Reset LCD
    SET_BIT(PORTD, SCEPIN);   // Tell LCD we're not sending data.
    SET_BIT(PORTB, RSTPIN);   // Stop resetting LCD

    LCD_CMD(lcd_set_function, lcd_instr_extended);
    LCD_CMD(lcd_set_contrast, contrast);
    LCD_CMD(lcd_set_temp_coeff, 0);
    LCD_CMD(lcd_set_bias, 3);

    LCD_CMD(lcd_set_function, lcd_instr_basic);
    LCD_CMD(lcd_set_display_mode, lcd_display_normal);
    LCD_CMD(lcd_set_x_addr, 0);
    LCD_CMD(lcd_set_y_addr, 0);
}

uint8_t jerry_bitmap[8] = {
    0b10001000,
    0b11111000,
    0b10101000,
    0b01110000,
    0b00000000,
    0b00000000,
    0b00000000,
    0b00000000
};


uint8_t jerry_direct[8];
uint8_t x, y;

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
            uint8_t bit_val = BIT_VALUE(jerry_bitmap[j], (7 - i));
            WRITE_BIT(jerry_direct[i], j, bit_val);
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
        LCD_DATA(jerry_direct[i]);
    }
}

/*
**  Draw character directly to LCD.
**  Bypasses the screen buffer used by <lcd.h>.
**
**	Parameters:
**      x, y:   Integer coordinates at which the character will be placed.
**              The y-position will be truncated to the nearest multiple of 8.
**      ch:     The character to draw.
**      colour: The colour. If FG_COLOUR, the characters are rendered normally.
**              If BG_COLOUR, the text is drawn as a negative (inverse).
**  Returns:
**      No result is returned.
*/
void draw_char_direct(int x, int y, char ch, colour_t colour) {
    // Do nothing if character does not fit on LCD.
    if (x < 0 || x > LCD_X - CHAR_WIDTH || y < 0 || y > LCD_Y - CHAR_HEIGHT) {
        return;
    }

    // Move LCD cursor to starting spot.
    LCD_CMD(lcd_set_function, lcd_instr_basic | lcd_addr_horizontal);
    LCD_CMD(lcd_set_x_addr, (x & 0x7f));
    LCD_CMD(lcd_set_y_addr, (y & 0x7f) / 8);

    // Send pixel blocks.
    for (int i = 0; i < CHAR_WIDTH; i++) {
        uint8_t pixelBlock = pgm_read_byte(&(ASCII[ch - ' '][i]));

        if (colour == BG_COLOUR) {
            pixelBlock = ~pixelBlock;
        }

        LCD_DATA(pixelBlock);
    }
}

/*
**	Draw string directly to LCD.
**  Bypasses the screen buffer used by <lcd.h>.
**
**	Parameters:
**      x, y:   Integer coordinates at which the character will be placed.
**              The y-position will be truncated to the nearest multiple of 8.
**      str:    The text to draw.
**      colour: The colour. If FG_COLOUR, the characters are rendered normally.
**              If BG_COLOUR, the text is drawn as a negative (inverse).
**  Returns:
**      No result is returned.
*/
void draw_string_direct(int x, int y, char * str, colour_t colour) {
    for (int i = 0; str[i] != 0; i++, x += CHAR_WIDTH) {
        draw_char_direct(x, y, str[i], colour);
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

void setup(void) {
    set_clock_speed(CPU_8MHz);
    new_lcd_init(LCD_LOW_CONTRAST);
    lcd_clear();
    draw_string_direct((LCD_X - 6 * CHAR_WIDTH) / 2, 4 * CHAR_HEIGHT, "Direct", FG_COLOUR);
    draw_string_direct((LCD_X - 6 * CHAR_WIDTH) / 2, 5 * CHAR_HEIGHT, "Writer", FG_COLOUR);
    setup_smiley();
    draw_smiley();
}

void process(void) {
    erase_smiley();
    x = (x + 1) % (LCD_X - 8);
    draw_smiley();
}

int main(void) {
    setup();

    for (;; ) {
        process();
        _delay_ms(100);
    }
}