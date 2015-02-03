#include <prodcons.h>

void producer(int count)
{
	int i;
	//Code to produce values less than equal to count, 
	for ( i = 1; i <= count; i++)
	{
		//produced value should get assigned to global variable 'n'.
		n=i;
		//print produced value e.g. produced : 8
		printf("\nproduced : %d", n);
	}
}

