/*
 * @file     xsh_prodcons.c
 *
 */
#include <prodcons.h>
#include <future.h>

/* Global Variable n used as shared memory by producer and consumer.*/
int n;
//semaphore consumed,produced;

/**
 * @ingroup shell
 *
 * Shell command (prodcons).
 * @param nargs  number of arguments in args array
 * @param args   array of arguments
 * @return OK for success, SYSERR for syntax error
 */
shellcmd xsh_prodcons(int nargs, char *args[])
{
    int count = 2000;
    int future_flag = 0;
    semaphore produced,consumed;

    /* Output help, if '--help' argument was supplied */
    if (nargs == 2 && strcmp(args[1], "--help") == 0)
    {
        printf("Usage: %s <flag> | <positive number>\n\n", args[0]);
        printf("Description:\n");
        printf("\tIllustration of Producer-Consumer model.\n");
        printf("Options:\n");
        printf("\t--help\tdisplay this help and exit\n");
        printf("flag:\n");
        printf("\t-f\tuse futures\n");
        return OK;
    }
        
    /* Check for correct number of arguments */
    if (nargs > 2)
    {
	fprintf(stderr, "%s: too many arguments\n", args[0]);
        fprintf(stderr, "Try '%s --help' for more information\n",
                    args[0]);
        return SYSERR;
    }

    if (nargs == 2)
    {
        if(strcmp(args[1],"-f")==0)
          future_flag = 1;
	else
          count = atoi(args[1]);
    }

    if(future_flag == 1)
    {
  	future  *f_exclusive, *f_shared, *f_queue;
  	f_exclusive = future_alloc(FUTURE_EXCLUSIVE);
  	f_shared = future_alloc(FUTURE_SHARED);
 	f_queue = future_alloc(FUTURE_QUEUE);
 
	// Test FUTURE_EXCLUSIVE
	  resume( create(future_cons, 1024, 20, "fcons1", 1, f_exclusive) );
	  resume( create(future_prod, 1024, 20, "fprod1", 1, f_exclusive) );

	// Test FUTURE_SHARED
	  resume( create(future_cons, 1024, 20, "fcons2", 1, f_shared) );
	  resume( create(future_cons, 1024, 20, "fcons3", 1, f_shared) );
	  resume( create(future_cons, 1024, 20, "fcons4", 1, f_shared) ); 
	  resume( create(future_cons, 1024, 20, "fcons5", 1, f_shared) );
	  resume( create(future_prod, 1024, 20, "fprod2", 1, f_shared) );

	// Test FUTURE_QUEUE
	  resume( create(future_cons, 1024, 20, "fcons6", 1, f_queue) );
	  resume( create(future_cons, 1024, 20, "fcons7", 1, f_queue) );
	  resume( create(future_cons, 1024, 20, "fcons8", 1, f_queue) );
	  resume( create(future_cons, 1024, 20, "fcons9", 1, f_queue) );
	  resume( create(future_prod, 1024, 20, "fprod3", 1, f_queue) );
	  resume( create(future_prod, 1024, 20, "fprod4", 1, f_queue) );
	  resume( create(future_prod, 1024, 20, "fprod5", 1, f_queue) );
	  resume( create(future_prod, 1024, 20, "fprod6", 1, f_queue) );
	  
	  if(future_free(f_exclusive)==SYSERR)
		kprintf("future_free failed\n");
	  if(future_free(f_shared)==SYSERR)
		kprintf("future_free failed for shared \n");
	  if(future_free(f_queue)==SYSERR)
		kprintf("future_free failed\n");

    }
    else
    {
	/* Check if argument is a number */
	if (count < 1 || count > UINT_MAX)
	{
		fprintf(stderr, "%s: accepts flag or positive numbers only\n", args[0]);
		fprintf(stderr, "Try '%s --help' for more information\n", args[0]);
		return SYSERR;
	}

	/*Initialise Semaphores*/
	consumed = semcreate(1);
	produced = semcreate(0);

	resume( create(producer, 1024, 20, "producer",3,consumed,produced,count) );
	resume( create(consumer, 1024, 20, "consumer",3,consumed,produced,count) );

    }
    return OK;
}
