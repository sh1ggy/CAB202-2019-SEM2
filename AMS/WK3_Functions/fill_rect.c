#include <stdlib.h>
#include <cab202_graphics.h>

//  Insert your solution here.
void fill_rect ( int left, int top, int right, int bottom, char ch ) {
	int width = right - left + 1;
	int height = bottom - top + 1;
	if ( width < 1 || height < 1 ) {
		return;
	}
	else {
		// draws each line by progressively by using the iteration variable
		for (int i = 0; i < height; i++) {
			draw_line(left, top + i, right, top + i, ch);
		}
	}
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
	fill_rect( l, t, r, b, c );
	show_screen();
	wait_char();

	return 0;
}
