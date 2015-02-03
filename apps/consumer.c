#include <prodcons.h>

void consumer(int count)
{
	int i;
	//Code to consume values until less than equal to count, 
	for ( i = 1; i <= count; i++)
	{
		//consume global variable 'n'.
		//print consumed value e.g. consumed : 8
		printf("\nconsumed : %d", n);
	}
}

