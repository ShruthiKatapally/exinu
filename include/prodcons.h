#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>   
#include <shell.h>
#include <limits.h>
#include <semaphore.h>

/*Global variable for producer consumer*/
extern int n; /*this is just declaration*/

/*Declare the required semaphores*/
extern semaphore consumed, produced;

/*function Prototype*/
void producer(semaphore consumer,semaphore producer,int count);
void consumer(semaphore consumer,semaphore producer,int count);
