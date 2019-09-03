#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <cab202_graphics.h>
#include <cab202_timers.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846264338327950288 // pi
#endif


bool game_over = false; // Set this to true when game is over
bool game_pause = false; // Set this to true when game is paused
bool start_game = true; // Used for restarting game
bool next_level = true; // Set this to true when the next level has to be spawned


// Setting up timers and TIME variables
timer_id cheese_timer;
timer_id trap_timer;
timer_id seconds_timer;
int seconds;
int minutes;

// JERRY state
double jerry_x, jerry_y, jerry_startx, jerry_starty;
#define JERRY_IMG 'J'
#define LIVES 5
int lives = LIVES;

// TOM state
double tom_x, tom_y, tom_dx, tom_dy, tom_startx, tom_starty;
#define TOM_IMG 'T'

// CHEESE state
double cheese_y, cheese_x;
int score = 0;
int cheese_count = 0;
int cheese_index = 0;
double cheese_coords[5][2];
#define CHEESE_IMG 'C'

// TRAP state
double trap_y, trap_x;
int trap_count = 0;
int trap_index = 0;
double trap_coords[5][2];
#define MOUSETRAP_IMG 'M'

// Screen dimensions
int W, H;

// WALL state
double wall_coords[50][4];
int num_walls = 0;
#define WALL_IMG 'W'

// DOOR state
double door_x, door_y;
bool door_flag = false;
int door_count = 0;
#define DOOR_IMG 'X'

// FIREWORK state
double weap_step_x, weap_step_y, weap_x, weap_y;
int weap_count;
#define WEAPON_IMG 'F'

// Room change state
int count_arg;
int num_room = 1;

/**
 *  Restarts all gamestates
 */
void restart()
{
    memset(cheese_coords, -1, sizeof(cheese_coords)); 
    memset(trap_coords, -1, sizeof(trap_coords));
    cheese_timer = create_timer(2000);
    trap_timer = create_timer(3000);
    num_room = 1;
    next_level = true;

    tom_x = tom_startx;
    tom_y = tom_starty;

    jerry_x = jerry_startx;
    jerry_y = jerry_starty;

    score = 0; 
    lives = 5;
    cheese_count = 0;
    trap_count = 0;
    
    game_pause = false;
    seconds = 0;
    minutes = 0;
}

/**
 * UI:
 *  Shows the Game Over Screen
 *      (q key) quits to terminal
 *      (r key) restarts to level 1
 */
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
    
    while (true) {
        int key;
        key = get_char();
        if (key == 'r')
        {
            start_game = true;
            restart();
            break;
        }
        if (key == 'q')
        {
            start_game = false;
            game_over = true;
            break;
        }
    }
}

/**
 * UI:
 *	Draw top border using '=' symbols.
 */
void draw_border()
{
    const int ch = '=';
    draw_line(0, 3, W - 1, 3, ch);
}

/**
 * UI:
 *	Draw the status information.
 */
void draw_status()
{
    draw_formatted(0, 0, "Student Number: 10232117,");
    draw_formatted(26, 0, "Score: %2d,", score);
    draw_formatted(37, 0, "Lives: %2d,", lives);
    draw_formatted(48, 0, "Player: J,");
    draw_formatted(59, 0, "Time Elapsed: %02d:%02d", minutes, seconds);
    draw_formatted(0, 2, "Cheese: %2d,", cheese_count);
    draw_formatted(12, 2, "Mousetraps: %2d,", trap_count);
    draw_formatted(28, 2, "Weapons: %d,", weap_count);
    draw_formatted(40, 2, "Cheese Until Next Level: %2d", 5-door_count);
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

/**
 *  Reads the contents of the file and stores variables
 *  
 *  Parameters:
 *  (FILE *stream)  a pointer to the file data stored in the stream
 */
void read_file(FILE *stream)
{
    // Setting up the arguments to parse
    char command;
    double arg1, arg2, arg3, arg4;

    // Looping through file argument
    while (!feof(stream))
    {
        // Scanning the arguments within the text file
        double items_scanned = fscanf(stream, "%c %lf %lf %lf %lf", &command, &arg1, &arg2, &arg3, &arg4);
        
        // Setting up walls
        if (items_scanned == 5)
        {
            if (command == WALL_IMG)
            {
                // Putting scanned and scaled coordinates into an array
                wall_coords[num_walls][0] = round(arg1 * (W - 1));
                wall_coords[num_walls][1] = round(arg2 * (H - 1 - 3) + 4);
                wall_coords[num_walls][2] = round(arg3 * (W - 1));
                wall_coords[num_walls][3] = round(arg4 * (H - 1 - 3) + 4);
                // Incrementing the array
                num_walls++;
            }
        }
        // Setting up Jerry and Tom
        else if (items_scanned == 3)
        {
            if (command == JERRY_IMG)
            {
                // Scaling Jerry (x,y) coords to the screen 
                jerry_x = round(arg1 * (W - 1));
                jerry_startx = round(jerry_x);
                jerry_y = round(arg2 * (H - 4) + 4);
                jerry_starty = round(jerry_y);
            }
            if (command == TOM_IMG)
            {
                // Scaling Tom (x,y) coords to the screen
                tom_startx = round(arg1 * (W - 1));
                tom_x = round(tom_startx);
                tom_starty = round(arg2 * (H - 1 - 4) + 4);
                tom_y = round(tom_starty);
            }
        }
    }
}

/** 
 * JERRY:
 *  Sets up Jerry's (x,y) coords to the initial ones read in the file
 */
void setup_jerry()
{
    jerry_x = jerry_startx;
    jerry_y = jerry_starty;
}

/**
 * JERRY:
 *	Updates the position of Jerry based on a key code.
 *
 *	Parameter: ch, the key code to process.
 */
void update_jerry(int ch)
{
    // Checking the next move Jerry is going to make and preventing it if it's a wall
    if (ch == 'a' && jerry_x > 0 && scrape_char(jerry_x - 1, jerry_y) != WALL_IMG)
    {
        round(jerry_x--);
    }
    else if (ch == 'd' && jerry_x < W - 1 && scrape_char(jerry_x + 1, jerry_y) != WALL_IMG)
    {
        round(jerry_x++);
    }
    else if (ch == 's' && jerry_y < H - 1 && scrape_char(jerry_x, jerry_y + 1) != WALL_IMG)
    {
        round(jerry_y++);
    }
    else if (ch == 'w' && jerry_y - 1 >= 4 && scrape_char(jerry_x, jerry_y - 1) != WALL_IMG)
    {
        round(jerry_y--);
    }
}

/**
 * [SETUP] DOOR:
 *  Sets up the coordinates of the door.
 */
void setup_door() {
    door_x = round(1 + rand() % (W - 1));
    door_y = round(5 + rand() % (H - 5));
    // Checks for empty spaces
    while (true) {
        if (scrape_char(door_x, door_y) == WALL_IMG ||
            scrape_char(door_x, door_y) == CHEESE_IMG ||
            scrape_char(door_x, door_y) == MOUSETRAP_IMG) {
                door_x = round(1 + rand() % (W - 1));
                door_y = round(5 + rand() % (H - 5));    
        }
        else {
            break;
        }
    }

}

/**
 * [DRAW] DOOR:
 *  Draws door.
 */
void draw_door() {
    if (door_count == 5) {
        draw_char(door_x, door_y, DOOR_IMG);
    }
}

/**
 * [UPDATE] DOOR:
 *  Checks collision with Jerry 
 */
void update_door() {
    if (collided(jerry_x, jerry_y, door_x, door_y) && door_count == 5) {
        num_room++;
        if (num_room == count_arg) {
            game_over_method();
        }
        else {
            next_level = true;
            setup_door();
        }
        
    }
}

/**
 * [SETUP] CHEESE:
 *	Sets up the cheese.
 */
void setup_cheese()
{
    for (int i = 0; i < 5; i++) {
        cheese_coords[i][0] = -5;
        cheese_coords[i][1] = -5;
    }
}

/**
 * [DRAW] CHEESE:
 *	Draws the cheese.
 */
void draw_cheese()
{
    for (int i = 0; i < 5; i++) {
        draw_char(round(cheese_coords[i][0]), round(cheese_coords[i][1]), CHEESE_IMG);
    } 
}

/**
 * [UPDATE] CHEESE:
 *  Updates the state of the cheese, checking for collision with Jerry and
 *  if necessary incrementing the score and re-spawning the cheese.
 */
void update_cheese(int key)
{
    // Making sure 2 seconds pass and there is not more than 5 cheese onscreen 
    if (timer_expired(cheese_timer) && cheese_count < 5) {
        // Generate random coordinates for the appropriate index in the array
        cheese_coords[cheese_index][0] = round(1 + rand() % (W - 1));
        cheese_coords[cheese_index][1] = round(5 + rand() % (H - 5));

        while(true) {
            // Checking cheese coordinates for empty space 
            if (scrape_char(cheese_coords[cheese_index][0], cheese_coords[cheese_index][1]) == WALL_IMG ||
            scrape_char(cheese_coords[cheese_index][0], cheese_coords[cheese_index][1]) == MOUSETRAP_IMG) {
                // Reinitialising cheese coordinates
                cheese_coords[cheese_index][0] = round(1 + rand() % (W - 1));
                cheese_coords[cheese_index][1] = round(5 + rand() % (H - 5));
            } 
            else {
                break;
            }
        }

        // Incrementing the appropriate cheese counters
        cheese_index++;
        cheese_count++;
    }
    
    
    for (int i = 0; i < cheese_count; i++) {
        // Checking if Jerry has collided with any of the cheese, redrawing the cheese offscreen 
        if (collided(jerry_x, jerry_y, cheese_coords[i][0], cheese_coords[i][1])) {
            cheese_timer = create_timer(2000);
            score++;
            cheese_count--;
            if (door_count < 5) {
                door_count++;
            }
            
            cheese_coords[i][0] = -5;
            cheese_coords[i][1] = -5;
        }
        // If the cheese has been drawn offscreen, reinitialise the coordinates
        if (cheese_coords[i][0] == -5 && cheese_coords[i][1] == -5) {
            cheese_index = i;
        }
    }
}

/**
 * [SETUP] TOM:
 *	Sets up Tom at a random location and direction.
 */
void setup_tom()
{
    tom_x = tom_startx;
    tom_y = tom_starty;
    double tom_dir = rand() * M_PI * 2 / RAND_MAX;
    const double step = 0.2;

    tom_dx = step * cos(tom_dir);
    tom_dy = step * sin(tom_dir);
}

/**
 * TOM:
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

    if (new_x == 0 || new_x == W || scrape_char((tom_x+tom_dx), (tom_y)) == WALL_IMG )
    {
        // Bounce of left or right wall: reverse horizontal direction
        tom_dx = -tom_dx;
        bounced = true;
    }

    if (new_y == 4 || new_y == H || scrape_char((tom_x), (tom_y + tom_dy)) == WALL_IMG  )
    {
        // Bounce off top or bottom wall: reverse vertical direction'   double tom_dir = rand() * M_PI * 2 / RAND_MAX;
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

/**
 * [SETUP] TRAP:
 *	Fills the trap coordinates array so that the traps will draw them offscreen
 */
void setup_trap()
{
    for (int i = 0; i < 5; i++) {
        trap_coords[i][0] = -5;
        trap_coords[i][1] = -5;
    }
}

/**
 * [DRAW] TRAP:
 *	Draws all of the traps according to the coords in the trap_coords array
 */
void draw_trap()
{
    for (int i = 0; i < 5; i++) {
        draw_char(round(trap_coords[i][0]), round(trap_coords[i][1]), MOUSETRAP_IMG);
    } 
}

/**
 * [UPDATE] TRAP:
 *  Updates the state of the mousetraps, checking for collision with Jerry and
 *  if necessary decrementing the lives and re-spawning the mousetraps.
 */
void update_trap(int key)
{
    // Making sure 3 seconds pass and there is not more than 5 mousetraps onscreen
    if (timer_expired(trap_timer) && trap_count < 5) {
        // Generate random coordinates for the appropriate index in the array
        trap_coords[trap_index][0] = round(tom_x);
        trap_coords[trap_index][1] = round(tom_y);
        // Checking mousetrap coordinates, making sure they spawn in empty spaces
        if (scrape_char(trap_coords[trap_index][0], trap_coords[trap_index][1]) == WALL_IMG || 
        scrape_char(trap_coords[trap_index][0], trap_coords[trap_index][1]) == CHEESE_IMG) {
            // Reinitialising cheese coordinates
            trap_coords[trap_index][0]++;
            trap_coords[trap_index][1]++;
        }
        // Incrementing the appropriate mousetrap counters
        trap_index++;
        trap_count++;
    }
    
    // Looping through each of the coordinates in the mousetrap coords array
    for (int i = 0; i < trap_count; i++) {
        // Checking if Jerry has collided with any of the moustraps, redrawing the mousetraps offscreen
        if (collided(jerry_x, jerry_y, trap_coords[i][0], trap_coords[i][1])) {
            trap_timer = create_timer(3000);
            lives--;
            trap_count--;
            trap_coords[i][0] = -5;
            trap_coords[i][1] = -5;

            // Checking if a gameover is necessary
            if (lives <= 0)
            {
                game_over_method();
            }
        }
        // If any mousetrap has been drawn offscreen, reinitialise the coordinates
        if (trap_coords[i][0] == -5 && trap_coords[i][1] == -5) {
            trap_index = i;
        }
    }
}

/**
 * [UPDATE] TOM:
 *	Moves Tom (if it their turn), and checks for collision
 *	with Jerry.
 */
void update_tom(int key)
{
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
    else if (collided(weap_x, weap_y, tom_x, tom_y)) {
        weap_x = -5;
        weap_y = -5;
        setup_tom();
    }
    else if (key < 0 && !game_pause)
    {
        move_tom();
    }

}

/**
 * [DRAW] WALLS:
 *  Loops through the wall_coords array and draws lines between each of the 
 *  (x1,y1), (x2,y2) coordinates
 * 
 *  Parameter: stream, address of a FILE object which provides access to the stream
 *
 */
void draw_walls()
{ 
    for (int i = 0; i < num_walls; i++)
    {
        draw_line(wall_coords[i][0], wall_coords[i][1], wall_coords[i][2], wall_coords[i][3], WALL_IMG);
    }
}

/**
 * [DRAW] TOM:
 *  Draws Tom.
 */
void draw_tom() {
    draw_char(tom_x, tom_y, TOM_IMG);
}

/**
 * [DRAW] JERRY:
 *  Draws Jerry.
 */
void draw_jerry() {
    draw_char(jerry_x, jerry_y, JERRY_IMG);
}

/**
 * [SETUP] WEAPON:
 *  Sets up the weapon.
 */
void setup_weap() {
    weap_x = -5;
    weap_y = -5;
}

/**
 * WEAPON:
 *  Initialises the weapon to Jerry's current coordinates.
 */
void init_weap() {
    weap_x = jerry_x;
    weap_y = jerry_y;
}

/**
 * [DRAW] JERRY:
 *  Draws Jerry.
 */
void draw_weap() {
    draw_char(round(weap_x), round(weap_y), WEAPON_IMG); 
}

/**
 * [UPDATE] WEAPON:
 *  Calculates the trajectory of the weapon towards Tom.
 *  Checks the collision of the weapon with the walls and the borders of the screen.
 */
void update_weap(int ch) {
    if (ch == 'f' && num_room >= 2 && weap_x == -5 && weap_y == -5) {
        weap_count++;
        init_weap();
        double t_1 = tom_x - jerry_x;
        double t_2 = tom_y - jerry_y;
        double distance = sqrt(t_1*t_1 + t_2*t_2);
        weap_step_x = t_1 * 0.2 / distance;
        weap_step_y = t_2 * 0.2 / distance;
        
    }
    double new_x = weap_x + weap_step_x;
    double new_y = weap_y + weap_step_y;
    if (new_x < 0 || new_x < W - 1 || new_y < H - 1 || new_y-1 <= 4 || 
    scrape_char(new_x, new_y) != WALL_IMG) {
        weap_x += weap_step_x;
        weap_y += weap_step_y;
    }
    if (scrape_char(new_x, new_y) == WALL_IMG || scrape_char(new_x+1, new_y+1) == 'W' || new_x < 0 || new_y > H - 1 || new_y <= 4) {
        weap_x = -5;
        weap_y = -5;
    }
    for (int i = 0; i < num_walls; i++) {
        if (collided(new_x, new_y, wall_coords[i][0], wall_coords[i][1]) || 
        collided(new_x, new_y, wall_coords[i][2], wall_coords[i][3])) {
            weap_x = -5;
            weap_y = -5;
        }
    }
}

/**
 * DRAW:
 *	Draws all objects in the game.
 */
void draw_all()
{
    clear_screen();
    draw_border();
    draw_status();
    draw_walls();

    draw_cheese();
    draw_trap();
    draw_tom();
    draw_jerry();
    draw_door();

    draw_weap();
    
    show_screen();
}

/**
 * SETUP:
 *  Sets up all objects in the game.
 */
void setup(void)
{
    srand(get_current_time());

    W = screen_width();
    H = screen_height();

    seconds_timer = create_timer(1000);
    cheese_timer = create_timer(2000);
    trap_timer = create_timer(3000);

    setup_cheese();
    setup_trap();
    setup_tom();
    setup_door();
    setup_weap();
}

/**
 * [UPDATE] QUIT GAME:
 *  Reads for the 'q' key and changes appropriate bool values for ending the game
 * 
 *  Parameter: key, the key code to process. 
 */
void update_gameover(int key)
{
    if (key == 'q')
    {
        game_over = true;
        start_game = false;
    }
}

/**
 * [UPDATE] PAUSE GAME:
 *  Reads for the 'p' key and changes the appropriate bool values 
 *  for pausing and unpausing the game.
 * 
 *  Parameter: key, the key code to process. 
 */
void update_pause(int key)
{
    if (key == 'p')
    {
        // If the game is not paused, pause
        if (!game_pause)
        {
            game_pause = true;
            return;
        }
        // If the game is paused, unpause
        else if (game_pause)
        {
            game_pause = false;
            return;
        }
    }
}

/**
 * [UPDATE] LEVEL:
 *  Loops through the levels, each time resetting the values 
 *  back to their defaults and if the last level has been reached
 *  end the game.
 * 
 *  Parameter: key, the key code to process. 
 */
void update_level(int key)
{
    if (key == 'l')
    {
        num_room++;
        if (num_room == count_arg) {
            game_over_method();
        }
        else if (num_room < count_arg) {
            next_level = true;
            cheese_count = 0;
            tom_x = tom_startx;
            tom_y = tom_starty;
            jerry_x = jerry_startx;
            jerry_y = jerry_starty;
        }
    }
}

/**
 * [LOOP]
 *  Updates the state of all objects in the game, based on user input and
 *  the existing state.
 */
void loop()
{
    int key = get_char();

    update_gameover(key);
    update_pause(key);
    update_level(key);
    update_door();

    update_jerry(key);
    update_cheese(key);
    update_tom(key);
    update_trap(key);

    update_weap(key);
}

/**
 * [SETUP] LEVEL
 *  Sets up for the next level by resetting appropriate game states
 */
void setup_level() {
    memset(wall_coords, -1, num_walls);
    num_walls = 0;

    memset(cheese_coords, -1, sizeof(cheese_coords));
    cheese_count = 0;
    cheese_index = 0;

    memset(trap_coords, -1, sizeof(trap_coords));
    trap_count = 0;
    trap_index = 0;

    door_count = 0;
}

/**
 * [FILE] WALLS:
 *  Checks if the room has to be overwritten and changed to 
 *  the next level 
 */
void restart_walls(int argc, char *argv[])
{
    if (next_level)
    {
        if (argc > 1)
        {
            FILE *stream = fopen(argv[num_room], "r");
            if (stream != NULL)
            {
                setup_level();
                read_file(stream);
                fclose(stream);
            }
        }
        next_level = false;
    }
}

/**
 * [TIME]
 *  Increments the time according to seconds and if the game isn't paused
 */
void increment_time() {
    if (timer_expired(seconds_timer) && !game_pause)
    {
        seconds++;
        if (seconds == 60)
        {
            seconds = 0;
            minutes++;
        }
    }
}

/**
 *  Sets up the game and hosts the event loop.
 */
int main(int argc, char *argv[])
{
    while (start_game) {
        const int DELAY = 10;

        setup_screen();
        setup();

        count_arg = argc; 
        while (!game_over)
        {
            restart_walls(argc, argv);
            loop();
            draw_all();

            increment_time();
            timer_pause(DELAY);
        }
    }
    return 0;
}