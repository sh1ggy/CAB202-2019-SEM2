#include <stdio.h>

#define CAPACITY	4

int main()
{
	int numbers[CAPACITY] = {19, 32, -435, 67};

	for (int i = 0; i < CAPACITY; i++)
	{
		printf("numbers[%d] == %d\n", i, numbers[i]);
	}

	return 0;
}