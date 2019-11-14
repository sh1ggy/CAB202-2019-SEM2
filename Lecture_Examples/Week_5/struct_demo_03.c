/**
 * struct_demo_01.c
 * Lawrence Buckingham, Queensland University of Technology.
 * 
 * Illustrates:
 *  Defining a struct.
 *  Reading data into the fields of a struct.
 *  Returning a struct from a function (by reference).
 *  Sending a struct to a function (by value).
 *  Writing the contents of a struct to a stream.
 *  Reading data into an array of structs.
 *  Sending an array of struct to a function.
 */
#include <stdio.h>
#include <stdbool.h>
#include <math.h>
#include <cab202_graphics.h>

#define MAX 10000

/**
 * Define a struct to hold a relative point location.
 */
struct point
{
    bool ok;  //< True if and only if the point is valid.
    double x; //< The horizontal position, relative to the terminal window
    double y; //< The vertical position of the point, relative to the terminal window.
};

/**
 * Attempt to read the coordinates of a point from a stream.
 * Parameters:
 *  f   - The address of a FILE object connected to the stream.
 * Returns:
 *  Returns a point struct p, such that p.ok is true if an only if
 * two valid floating point values were scanned, and both are between
 * 0.0 and 1.0 inclusive.  
 */
void read_point(FILE *f, struct point *p)
{
    int fields_scanned = fscanf(f, "%lf %lf", &p->x, &p->y);
    p->ok = (fields_scanned == 2) && (p->x >= 0) && (p->x <= 1) &&
            (p->y >= 0) && (p->y <= 1);
}

/**
 * Sends the contents of a point struct to the designated stream.
 * If the point is OK, the word "valid" is written, otherwise the word 
 * "invalid" is written. After this, the numeric values are written. 
 * 
 * Parameters:
 *  f   The address of a FILE object connected to the stream.
 *  p   A point to be sent to the stream.
 */
void write_point(FILE *f, struct point *p)
{
    fprintf(f, "%s: %f %f", (p->ok ? "valid" : "invalid"), p->x, p->y);
}

/**
 * Scales a point to the size of the terminal and draws a character at 
 * that position.
 * 
 * Parameters:
 *  p   The address of a point at which to draw.
 *  ch  The symbol to draw.
 */
void draw_point(struct point *p, int ch)
{
    if (p->ok)
    {
        int x = round(p->x * screen_width());
        int y = round(p->y * screen_height());
        draw_char(x, y, ch);
    }
}

/**
 * Renders a list of points, scaling each to the dimensions of the 
 * terminal.
 * 
 * Parameters:
 *  p   An array of points to draw.
 *  n   The number of points to draw.
 */
void draw_points(struct point p[], int n)
{
    for (int i = 0; i < n; i++)
    {
        draw_point(&p[i], '*');
    }
}

/**
 * main. This program reads a sequence of coordinate pairs from a file
 * and displays them, scaled to the terminal window.
 * 
 * Parameters:
 *  argc    An integer with the number of command line arguments.
 *  argv    An array of strings containing the command line arguments.
 */
int main(int argc, char *argv[])
{
    if (argc > 1)
    { // Process file.
        FILE *f = fopen(argv[1], "r");
        if (f)
        { // Draw contents of file.
            struct point p[MAX] = {{false, 0, 0}};
            int n;

            for (n = 0; n < MAX && !feof(f); n++)
            { // Scan points from file.
                read_point(f, &p[n]);
            }

            fclose(f);

            setup_screen();
            draw_points(p, n);
            show_screen();
            wait_char();
            return 0;
        }
        else
        { // Error opening file.
            fprintf(stderr, "Usage: %s input_file_name\n", argv[0]);
            fprintf(stderr, "Unable to open file '%s'.\n", argv[1]);
            return 1;
        }
    }
    else
    { // No file name supplied.
        fprintf(stderr, "Usage: %s input_file_name\n", argv[0]);
        fprintf(stderr, "Expected a file name on the command line.\n");
        return 1;
    }
}
