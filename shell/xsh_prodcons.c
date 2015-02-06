/*
 * @file     xsh_prodcons.c
 *
 */
#include <prodcons.h>

/* Global Variable n used as shared memory by producer and consumer.*/
int n;

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
    
    /* Output help, if '--help' argument was supplied */
    if (nargs == 2 && strcmp(args[1], "--help") == 0)
    {
        printf("Usage: %s <positive number>\n\n", args[0]);
        printf("Description:\n");
        printf("\tIllustration of Producer-Consumer model.\n");
        printf("Options:\n");
        printf("\t--help\tdisplay this help and exit\n");
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
	count = atoi(args[1]);
    }
    
    /* Check if argument is a number */
    if (count < 1 || count > UINT_MAX)
    {
	fprintf(stderr, "%s: accepts positive numbers only\n", args[0]);
        fprintf(stderr, "Try '%s --help' for more information\n",
                    args[0]);
        return SYSERR;
    }

    resume( create(producer, 1024, 20, "producer", 1, count) );
    resume( create(consumer, 1024, 20, "consumer", 1, count) );

    return OK;
}
