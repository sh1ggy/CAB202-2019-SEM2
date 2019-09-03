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
bool next_room = true;

// Setting up timers and time variables
timer_id cheese_timer;
timer_id seconds_timer;
int seconds;
int minutes;

// Jerry state
double jerry_x, jerry_y, jerry_startx, jerry_starty;
#define JERRY_IMG 'J'
#define LIVES 5
int lives = LIVES;

// Tom state
double tom_x, tom_y, tom_dx, tom_dy, tom_startx, tom_starty;
#define TOM_IMG 'T'

// Cheese state
double cheese_y, cheese_x;
int score, cheese_count;
//double cheese_coords[5][2];
#define CHEESE_IMG 'C'

// Screen dimensions
int W, H;

// Wall state
double wall_coords[50][4];
int num_walls = 0;
#define WALL_IMG 'W'

int count_arg;
int num_room = 1;

/**
 *	Draw a border around the screen using '*' symbols.
 */
void draw_border()
{
    const int ch = '=';
    draw_line(0, 3, W - 1, 3, ch);
}

/**
 *	Draw the status information.
 */
void draw_status()
{
    draw_formatted(0, 0, "Student Number: 10232117,");
    draw_formatted(26, 0, "Score: %d,", score);
    draw_formatted(36, 0, "Lives: %d,", lives);
    draw_formatted(46, 0, "Player: J,");
    draw_formatted(57, 0, "Time Elapsed: %02d:%02d", minutes, seconds);
    draw_formatted(0, 2, "Cheese: %d,", cheese_count);
    draw_formatted(11, 2, "Mousetraps: x,");
    draw_formatted(26, 2, "Weapons: x");
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
bool collided(double x0, double y0, double x1, double y1)
{
    return round(x0) == round(x1) && round(y0) == round(y1);
}

void read_file(FILE *stream)
{
    // Setting up the arguments to parse
    char command;
    double arg1, arg2, arg3, arg4;

    // Looping through file argument
    while (!feof(stream))
    {
        double items_scanned = fscanf(stream, "%c %lf %lf %lf %lf", &command, &arg1, &arg2, &arg3, &arg4);
        if (items_scanned == 5)
        {
            if (command == 'W')
            {
                // Putting scanned coordinates into an array
                wall_coords[num_walls][0] = arg1 * (W - 1);
                wall_coords[num_walls][1] = arg2 * (H - 1 - 3) + 4;
                wall_coords[num_walls][2] = arg3 * (W - 1);
                wall_coords[num_walls][3] = arg4 * (H - 1 - 3) + 4;
                num_walls++;
            }
        }
        // Setting up Jerry and Tom
        else if (items_scanned == 3)
        {
            if (command == 'J')
            {
                jerry_x = arg1 * (W - 1);
                jerry_y = arg2 * (H - 4) + 4;
                jerry_startx = jerry_x;
                jerry_starty = jerry_y;
            }
            if (command == 'T')
            {
                tom_startx = arg1 * (W - 1);
                tom_x = tom_startx;
                tom_starty = arg2 * (H - 1 - 4) + 4;
                tom_y = tom_starty;
            }
        }
    }
}

void setup_jerry()
{
    jerry_x = jerry_startx;
    jerry_y = jerry_starty;
}

/**
 *	Updates the position of Jerry based on a key code.
 *
 *	Parameter: ch, the key code to process.
 */
void update_jerry(int ch)
{
    if (ch == 'a' && jerry_x > 0 && scrape_char(jerry_x - 1, jerry_y) != WALL_IMG)
    {
        jerry_x--;
    }
    else if (ch == 'd' && jerry_x < W - 1 && scrape_char(jerry_x + 1, jerry_y) != WALL_IMG)
    {
        jerry_x++;
    }
    else if (ch == 's' && jerry_y < H - 1 && scrape_char(jerry_x, jerry_y + 1) != WALL_IMG)
    {
        jerry_y++;
    }
    else if (ch == 'w' && jerry_y - 1 >= 4 && scrape_char(jerry_x, jerry_y - 1) != WALL_IMG)
    {
        jerry_y--;
    }
}

/**
 *	Sets up the cheese.
 */
void setup_cheese()
{

    //srand(get_current_time() - 10000);

    // for (int i = 0; i <= 5; i++) {
    //     cheese_coords[i][0] = 1 + rand() % (W - 1);
    //     cheese_coords[i][1] = 5 + rand() % (H - 5);
    //     if (scrape_char(cheese_x, cheese_y) == WALL_IMG) {
    //         i--;
    //     }
    // }

    cheese_x = 1 + rand() % (W - 1);
    cheese_y = 5 + rand() % (H - 5);
    if (scrape_char(cheese_x, cheese_y) == WALL_IMG)
    {
        setup_cheese();
    }
}

/**
 *	Draws the cheese.
 */
void draw_cheese()
{
    draw_char(round(cheese_x), round(cheese_y), CHEESE_IMG);
}

/**
 *  Updates the state of the cheese, checking for collision with Jerry and
 *  if necessary incrementing the score and re-spawning the cheese.
 */
void update_cheese(int key)
{
    if (collided(jerry_x, jerry_y, cheese_x, cheese_y))
    {
        score++;
        cheese_count--;
        setup_cheese();
        cheese_timer = create_timer(2000);
    }
    else if (timer_expired(cheese_timer) && cheese_count < 5)
    {
        cheese_count++;
        setup_cheese();
    }
}

/**
 *	Sets up Tom at a random location and direction.
 */
void setup_tom()
{
    tom_x = tom_startx;
    tom_y = tom_starty;
    double tom_dir = rand() * M_PI * 2 / RAND_MAX;
    const double step = 0.7;

    tom_dx = step * cos(tom_dir);
    tom_dy = step * sin(tom_dir);
}

void draw_mousetrap()
{
    draw_char(round(tom_x), round(tom_y), 'M');
}

void setup_mousetrap()
{
}

/**
 *	Moves Tom a single step (if possible) with reflection
 *	from the border.
 */
void move_tom()
{
    // Assume that Tom has not already collided with the borders.
    // Predict the next screen position of Tom.
    int new_x = round(tom_x + tom_dx);
    int new_y = round(tom_y + tom_dy);

    bool bounced = false;
//round(new_x), round(tom_y)
    if (new_x == 0 || new_x == W || scrape_char((tom_x+tom_dx), (tom_y)) == 'W')
    {
        // Bounce of left or right wall: reverse horizontal direction
        double tom_dir = rand() * M_PI * 2 / RAND_MAX;
        const double step = 0.7;
        tom_dx = step * cos(tom_dir);
        tom_dx = -tom_dx;
        bounced = true;
    }
//round(tom_x), round(new_y)
    if (new_y == 4 || new_y == H || scrape_char((tom_x), (tom_y + tom_dy)) == 'W')
    {
        // Bounce off top or bottom wall: reverse vertical direction'   double tom_dir = rand() * M_PI * 2 / RAND_MAX;
        double tom_dir = rand() * M_PI * 2 / RAND_MAX;
        const double step = 0.7;
        tom_dy = step * sin(tom_dir);
        tom_dy = -tom_dy;
        bounced = true;
    }

    if (!bounced)
    {
        // no bounce: move instead.
        tom_x += tom_dx;
        tom_y += tom_dy;
    }
}

void game_over_method()
{
    clear_screen();

    const char *message[] = {
        "GAME OVER!",
        "Press r key to restart..."
        "Press q key to exit..."};

    const int rows = 2;

    for (int i = 0; i < rows; i++)
    {
        // Draw message in middle of screen.
        int len = strlen(message[i]);
        int x = (W - len) / 2;
        int y = (H - rows) / 2 + i;
        draw_formatted(x, y, message[i]);
    }

    show_screen();

    int key = get_char();
    wait_char();
    if (key == 'q')
    {
        game_over = true;
        return;
    }
    else if (key == 'r')
    {
        return;
    }
    //while (get_char() > 0) {}

    //game_over = true;
}

/**
 *	Moves Tom (if it their turn), and checks for collision
 *	with Jerry.
 */
void update_tom(int key)
{
    if (key < 0 && !game_pause)
    {
        move_tom();
    }

    if (collided(jerry_x, jerry_y, tom_x, tom_y))
    {
        setup_tom();
        setup_jerry();
        lives--;

        if (lives <= 0)
        {
            game_over_method();
        }
    }
}



/**
 *  Draws the starting position of Tom, Jerry and the level according to the provided 
 *  text file
 * 
 *  Parameters:
 *      stream - address of a FILE object which provides access to the stream
 *
 */
void draw_file()
{
    
    for (int i = 0; i < num_walls; i++)
    {
        draw_line(wall_coords[i][0], wall_coords[i][1], wall_coords[i][2], wall_coords[i][3], WALL_IMG);
    }
    draw_char(tom_x, tom_y, TOM_IMG);
    draw_char(jerry_x, jerry_y, JERRY_IMG);
}

/**
 *	Draws the display.
 */
void draw_all()
{
    clear_screen();
    draw_border();
    draw_status();
    draw_cheese();
    draw_file();
    show_screen();

}

/**
 *  Sets up all objects in the game.
 */
void setup(void)
{
    W = screen_width();
    H = screen_height();
    seconds_timer = create_timer(1000);
    cheese_timer = create_timer(2000);
    setup_cheese();
    setup_tom();
}

void update_gameover(int key)
{
    if (key == 'q')
    {
        game_over = true;
        return;
    }
}

void update_pause(int key)
{
    if (key == 'p')
    {
        if (!game_pause)
        {
            game_pause = true;
            return;
        }
        else if (game_pause)
        {
            game_pause = false;
            return;
        }
    }
}

void update_level(int key)
{
    if (key == 'l')
    {
        num_room++;
        next_room = true;
        cheese_count = 0;
        tom_x = tom_startx;
        tom_y = tom_starty;
        jerry_x = jerry_startx;
        jerry_y = jerry_starty;
    }
}

/**
 *  Updates the state of all objects in the game, based on user input and
 *  the existing state.
 */
void loop()
{
    int key = get_char();

    update_gameover(key);
    update_pause(key);
    update_level(key);

    update_jerry(key);
    update_cheese(key);
    update_tom(key);
}



void restart_walls(int argc, char *argv[])
{
    if (next_room)
    {
        if (argc > 1)
        {
            FILE *stream = fopen(argv[num_room], "r");
            if (stream != NULL)
            {
                memset(wall_coords, 0, num_walls);
                num_walls = 0;
                read_file(stream);
                fclose(stream);
            }
        }
        next_room = false;
    }
}

/**
 *  Sets up the game and hosts the event loop.
 */
int main(int argc, char *argv[])
{
    const int DELAY = 10;

    setup_screen();
    setup();

    count_arg = argc;
    while (!game_over)
    { // change back to !game_over
        restart_walls(argc, argv);
        draw_all();
        loop();

        // Updating timers
        if (timer_expired(seconds_timer) && !game_pause)
        {
            seconds++;
            if (seconds == 60)
            {
                seconds = 0;
                minutes++;
            }
        }
        timer_pause(DELAY);
    }

    return 0;
}