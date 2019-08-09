#include <cab202_graphics.h>
#include <cab202_timers.h>

//  Begin the definition of a function.
//  The name of the function is charry_night.
//  The function returns no result.
//  The function has the following formal parameters:
//    point_x - an array of int.
//    point_y - an array of int.
//    point_count - an int which specifies the number of symbols to draw.

void charry_night(int point_x[], int point_y[], int point_count) {
    //  Clear the screen but do not let the user see any changes yet.
    clear_screen();
    
    // Set up a counter controlled loop that will iterate over the valid contents
    // of the parallel arrays, and render a '^' at position (point_x[t],point_y[t])
    // for each t in [0..(point_count-1)].
    for(int i = 0; i<= point_count - 1; i++) {
        draw_char(point_x[i], point_y[i], '^');
    }
    //  Show the contents of the updated screen.
    show_screen();
}



   

// End the definition of the function.

#include <stdlib.h>
#include <time.h>

// Helper functions used by test driver 
void draw_test_pattern(void);
void random_fill_array(int x[], int n, int max);

// Minimal test driver. Modify this to carry out tests.
int main() {
    setup_screen();
    const int WIDTH = screen_width();
    const int HEIGHT = screen_height();

    draw_test_pattern();

    // Simulate the test setup process.
    srand(time(NULL));

    const int MAX_ITEMS = 200;
    const int MIN_ITEMS = 20;

    int x_coord[MAX_ITEMS];
    int y_coord[MAX_ITEMS];

    random_fill_array(x_coord, MAX_ITEMS, WIDTH);
    random_fill_array(y_coord, MAX_ITEMS, HEIGHT);

    int num_items = MIN_ITEMS + rand() % (MAX_ITEMS - MIN_ITEMS);

    for (int i = 0; i < num_items; i++) {
        x_coord[i] = rand() % WIDTH;
        y_coord[i] = rand() % HEIGHT;
    }

    // Call submitted code.
    charry_night(x_coord, y_coord, num_items);

    timer_pause(5000);
    return 0;
}

void draw_test_pattern() {
    const int MESS = 1000;
    srand(692673);

    for (int i = 0; i < MESS; i++) {
        draw_char(rand() % screen_width(), rand() % screen_height(), 'a' + rand() % 26);
    }

    show_screen();
}

void random_fill_array(int x[], int n, int max) {
    for (int i = 0; i < n; i++) {
        x[i] = rand() % max;
    }
}
