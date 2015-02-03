#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>   
#include <shell.h>

/*Global variable for producer consumer*/
extern int n; /*this is just declaration*/

/*function Prototype*/
void producer(int count);
void consumer(int count);
