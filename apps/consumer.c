#include <prodcons.h>

void consumer(semaphore consumed,semaphore produced,int count)
{
	int i;
	//Code to consume values until less than equal to count, 
	for ( i = 1; i <= count; i++)
	{
		wait(produced);
		//consume global variable 'n'.
		//print consumed value e.g. consumed : 8
		printf("\nconsumer: iteration = %d, value = %d \n", i, n);
		signal(consumed);
	}
	printf("\n");
}

