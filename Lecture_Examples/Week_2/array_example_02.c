#include <stdio.h>

#define CAPACITY 100

// Declare an array called values with capacity for CAPACITY values
// of the default floating point type.
double values[CAPACITY];

int main()
{
	// Declare integer variable to hold number of values, initialised to zero.
	int num_values = 0;

	// Get number of values
	printf("Please enter the number of values (from 0 to %d inclusive):\n", CAPACITY);
	scanf("%d", &num_values);

	// Ensure that number of values is between 0 and CAPACITY inclusive.
	if (num_values < 0 ) {
		num_values = 0;
	}

	if (num_values > CAPACITY) {
		num_values = CAPACITY;
	}

	// Get values from standard input stream
	for (int i = 0; i < num_values; i++)
	{
		printf("values[%d] = ?\n", i);
		scanf("%lf", &values[i]);
	}

	// Compute sum of values. Use a floating point variable to 
	//	accumulate the sum.
	double sum = 0;

	for ( int i = 0; i < num_values; i ++ ) {
		sum += values[i];
	}

	// Display the results.
	printf("Sum of %d values is %f.\n", num_values, sum);

	return 0;
}
