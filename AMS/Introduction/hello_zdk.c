#include <cab202_graphics.h>
#include <cab202_timers.h>

int main ( void )
{
    setup_screen();

    draw_string(10, 20, "Hello World!");
    show_screen();
    timer_pause(3000);

    cleanup_screen();
    return 0;
}