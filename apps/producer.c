#include <prodcons.h>

void producer(semaphore consumed,semaphore produced,int count)
{
	int i;
	//Code to produce values less than equal to count, 
	for ( i = 1; i <= count; i++)
	{
	wait (consumed);
	//produced value should get assigned to global variable 'n'.
	n=i;
	printf("producer: iteration = %d, value = %d ",i,n);
	signal(produced);
	}
}

