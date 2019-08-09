#include <stdlib.h>
#include <cab202_graphics.h>

//  Begin the definition a function called draw_rect that returns nothing, 
//  but requires the following arguments:
//
//      left - an int that specifies the absolute horizontal location 
//          of the left hand side of the rectangle.
//
//      top - an int that specifies the absolute vertical location 
//          of the upper edge of the rectangle.
//
//      right - an int that specifies the absolute horizontal location 
//          of the right hand side of the rectangle.
//
//      bottom - an int that specifies the absolute vertical location 
//          of the lower edge of the rectangle.
//
//      ch - a character value that will be used to draw the
//          display the rectangle.

void draw_rect ( int left, int top, int right, int bottom, char ch )
{
	int width = right - left + 1; // + 1 to include the value lost by finding the difference
	int height = bottom - top + 1; // positive is down in the terminal window

    //  Test to see if either the width or height of the rectangle is less 
    //  than 1. If so, the function should stop immediately and draw nothing.
	if ( width < 1 || height < 1 ) { // + 1 to include the differential 
		return;
	}

/*
	T	T
L			R

L			R
	B	B
*/

    //  Draw a horizontal line from left to right at the top, using the 
    //  display char.
	draw_line(left, top, right, top, ch); // (L,T)(R,T)

    //  Draw a horizontal line from left to right at the bottom, using the 
    //  display char.
	draw_line(left, bottom, right, bottom, ch);

    //  Draw a horizontal line from top to bottom at the left, using the 
    //  display char.
	draw_line(left, top, left, bottom, ch);

    //  Draw a horizontal line from top to bottom at the right, using the 
    //  display char.
	draw_line(right, top, right, bottom, ch);

}


int main( void ) {
	int l, t, r, b;
	char c;

	printf( "Please enter the horizontal location of the left edge of the rectangle: " );
	scanf( "%d", &l );

	printf( "Please enter the vertical location of the top edge of the rectangle: " );
	scanf( "%d", &t );

	printf( "Please enter the horizontal location of the right edge of the rectangle: " );
	scanf( "%d", &r );

	printf( "Please enter the vertical location of the bottom edge of the rectangle: " );
	scanf( "%d", &b );

	printf( "Please enter the character used to draw the rectangle? " );
	scanf( " %c", &c );

	setup_screen();
	draw_rect( l, t, r, b, c );
	show_screen();
	wait_char();

	return 0;
}
