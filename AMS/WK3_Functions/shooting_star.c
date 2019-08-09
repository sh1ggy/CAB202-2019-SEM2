#include <stdlib.h>
#include <math.h>
#include <cab202_graphics.h>
#include <cab202_timers.h>

bool game_over = false;

//  Declare global variables.
double x, y, dx, dy = 0;

void setup(void) {
    x = 9;
    y = 9;
    double t_1 = 71 - x;
    double t_2 = 18 - y;
    double d = sqrt(t_1*t_1 + t_2*t_2);
    dx = t_1 * 0.25 / d;
    dy = t_2 * 0.25 / d;
    draw_char(9,9,'~');
    draw_char(71,18,'*');
}

void loop(void) {
    x += dx;
    y += dy;
    clear_screen();
    draw_char(71,18,'*');
    draw_char(round(x),round(y),'~');
    if (round(x) == 71 && round(y) == 18) {
        game_over = true;
    } else {
        game_over = false;
    }
    show_screen();
}

int main() {
    // milliseconds sleep between calls to loop.
    const int DELAY = 10;

    setup_screen();
    setup();
    show_screen();

    while (!game_over) {
        loop();
        timer_pause(DELAY);
    }

    return 0;
}
