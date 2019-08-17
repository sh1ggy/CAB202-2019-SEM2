#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <cab202_graphics.h>
#include <cab202_timers.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288 // pi
#endif

// Set this to true when simulation is over
bool game_over = false;
bool game_pause = false;

// Setting up timers and time variables
timer_id cheese_timer;
int seconds;

// Jerry state
double jerry_x, jerry_y;
int lives;
#define JERRY_IMG 'J'
#define LIVES 5

// Tom state
double tom_x, tom_y, tom_dx, tom_dy;
#define TOM_IMG 'T'

// cheese state
double cheese_y, cheese_x;
int score;
int cheese_count;
#define CHEESE_IMG 'C'

// Screen dimensions
int W, H;

/**
 *	Draw a border around the screen using '*' symbols.
 */
void draw_border() {
    const int ch = '=';
    draw_line(0, 4, W - 1, 4, ch);
}

/**
 *	Draw the status information.
 */
void draw_status() {
    draw_formatted(10, 1, "Student Number: n10232117 Score: %d\t Lives: %d\t Active Player: J\t Time Elapsed: mm:ss\t", score, lives);
    draw_formatted(10, 3, "Cheese: %d Mousetraps: x\t Weapons: x\t", cheese_count);
}

/**
 *  Draws the starting position of Tom, Jerry and the level according to the provided 
 *  text file
 * 
 *  Parameters:
 *      stream - address of a FILE object which provides access to the stream
 *
 */
void draw_file (FILE * stream) {
    char pen = '*';
    while (!feof(stream)) {
        char command;
        double arg1, arg2, arg3, arg4; 
        double items_scanned = fscanf(stream, " %c %lf %lf %lf %lf", &command, &arg1, &arg2, &arg3, &arg4);

        if (items_scanned == 3) {
            if (command == JERRY_IMG) {
                pen = JERRY_IMG;
                draw_char(arg1, arg2, pen);
            }
            if (command == TOM_IMG) {
                pen = TOM_IMG;
                draw_char(arg1, arg2, pen);
            }
        }
        else if (items_scanned == 5) {
            if (command == 'W') {
                arg1 *= (screen_width() - 1);
                arg3 *= (screen_width() - 1);
                arg2 *= (screen_height() -1);
                arg4 *= (screen_height() -1);
                draw_line(arg1, arg2, arg3, arg4, pen);
            }
        }
    }
}

/**
 *  Detects collision between objects that each occupy a single pixel.
 *
 *  Parameters:
 *  (x0, y0)   the position of one object.
 *  (x1, y1)    the position of the other object.
 *
 *  Returns true if and only if the rounded locations are the same.
 */
bool collided(double x0, double y0, double x1, double y1) {
    return round(x0) == round(x1) && round(y0) == round(y1);
}

/**
 *	Sets up Jerry, placing it initially in the centre of the screen.
 */
void setup_jerry() {
    jerry_x = (W - 1) / 2;
    jerry_y = (H - 1) / 2;
    lives = LIVES;
}

/**
 *	Draws Jerry.
 */

void draw_jerry() {
    draw_char(round(jerry_x), round(jerry_y), JERRY_IMG);
}

/**
 *	Updates the position of Jerry based on a key code.
 *
 *	Parameter: ch, the key code to process.
 */
void update_jerry(int ch) {
    if (ch == 'a' && jerry_x > 1) {
        jerry_x--;
    }
    else if (ch == 'd' && jerry_x < W - 2) {
        jerry_x++;
    }
    else if (ch == 's' && jerry_y < H - 1) {
        jerry_y++;
    }
    else if (ch == 'w' && jerry_y > 5) {
        jerry_y--;
    }
}

/**
 *	Sets up the cheese.
 */
void setup_cheese() {
    cheese_x = 1 + rand() % (W - 2);
    cheese_y = 5 + rand() % (H - 5);
}

/**
 *	Draws the cheese.
 */
void draw_cheese() {
    draw_char(round(cheese_x), round(cheese_y), CHEESE_IMG);
}

/**
 *  Updates the state of the cheese, checking for collision with Jerry and
 *  if necessary incrementing the score and re-spawning the cheese.
 */
void update_cheese(int key) {
    while (cheese_timer % 2 == 0 && cheese_count < 5) {
        if (collided(jerry_x, jerry_y, cheese_x, cheese_y)) {
            score++;
            cheese_count--;
            setup_cheese();
        }
    }

}

/**
 *	Draws Tom.
 */

void draw_tom() {
    draw_char(round(tom_x), round(tom_y), TOM_IMG);
}

/**
 *	Sets up Tom at a random location and direction.
 */
void setup_tom() {
    tom_x = 1 + rand() % (W - 2);
    tom_y = 3 + rand() % (H - 4);

    double tom_dir = rand() * M_PI * 2 / RAND_MAX;
    const double step = 0.1;

    tom_dx = step * cos(tom_dir);
    tom_dy = step * sin(tom_dir);
}

void do_collided() {
    clear_screen();

    const char * message[] = {
        "Simulation over!",
        "Press any key to exit..."
    };

    const int rows = 2;

    for (int i = 0; i < rows; i++) {
        // Draw message in middle of screen.
        int len = strlen(message[i]);
        int x = (W - len) / 2;
        int y = (H - rows) / 2 + i;
        draw_formatted(x, y, message[i]);
    }

    show_screen();

    while (get_char() > 0) {}
    wait_char();

    game_over = true;
}

/**
 *	Moves Tom a single step (if possible) with reflection
 *	from the border.
 */
void move_tom() {
    // Assume that Tom is have not already collided with the borders.
    // Predict the next screen position of Tom.
    int new_x = round(tom_x + tom_dx);
    int new_y = round(tom_y + tom_dy);

    bool bounced = false;

    if (new_x == 0 || new_x == screen_width() - 1) {
        // Bounce of left or right wall: reverse horizontal direction
        tom_dx = -tom_dx;
        bounced = true;
    }

    if (new_y == 4 || new_y == screen_height()) {
        // Bounce off top or bottom wall: reverse vertical direction
        tom_dy = -tom_dy;
        bounced = true;
    }

    if (!bounced) {
        // no bounce: move instead.
        tom_x += tom_dx;
        tom_y += tom_dy;
    }
}

/**
 *	Moves Tom (if it their turn), and checks for collision
 *	with Jerry.
 */
void update_tom(int key) {
    if (key < 0) {
        move_tom();
    }

    if (collided(jerry_x, jerry_y, tom_x, tom_y)) {
        setup_tom();
        lives--;

        if (lives <= 0) {
            do_collided();
        }
    }
}

/**
 *	Draws the display.
 */
void draw_all() {
    clear_screen();
    draw_border();
    draw_status();
    draw_jerry();
    draw_cheese();
    draw_tom();
    show_screen();
}

/**
 *  Sets up all objects in the game.
 */
void setup(void) {
    srand(get_current_time());
    cheese_timer = create_timer(2000);
    W = screen_width();
    H = screen_height();
    setup_jerry();
    setup_cheese();
    setup_tom();
}

/**
 *  Updates the state of all objects in the game, based on user input and
 *  the existing state.
 */
void loop() {
    int key = get_char();

    if (key == 'q') {
        game_over = true;
        return;
    }
    if (key == 'p') {
        game_pause = true;
        return;
    }

    update_jerry(key);
    update_cheese(key);
    update_tom(key);
}

/**
 *  Sets up the game and hosts the event loop.
 */
int main(int argc, char *argv[]) {
    const int DELAY = 10;
    
    setup_screen();
    setup();


    while (!game_over) {
        draw_all();
        for (int i = 1; i < argc; i++) {
            FILE * stream = fopen(argv[i], "r"); // opening text file in read only
            if (stream != NULL) {
                draw_file(stream);
                fclose(stream);
            }
        }

        loop();
        timer_pause(DELAY);
    }

    return 0;
}