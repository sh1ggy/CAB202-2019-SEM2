#include <stdio.h> // literally inserting stuff in program

int main() {
    printf("Please enter tempersture in Fahrenheit:\n");
    double fahrenheit;
    // scanf() opposite of printf() 
    scanf( "%lf", &fahrenheit ); // % - long floating point, & into fahrenheit
    double celsius = 5.0 / 9.0 * (fahrenheit - 32);
    printf( "%f F is %f C.\n", fahrenheit, celsius );
    return 0;
}

