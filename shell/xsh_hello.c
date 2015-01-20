/*
 * @file     xsh_hello.c
 *
 */

#include <stddef.h>
#include <stdio.h>
#include <string.h>

/**
 * @ingroup shell
 *
 * Shell command (hello).
 * @param nargs  number of arguments in args array
 * @param args   array of arguments
 * @return OK for success, SYSERR for syntax error
 */
shellcmd xsh_hello(int nargs, char *args[])
{


    /* Output help, if '--help' argument was supplied */
    if (nargs == 2 && strcmp(args[1], "--help") == 0)
    {
        printf("Usage: %s <string>\n\n", args[0]);
        printf("Description:\n");
        printf("\tDisplays greetings with given string.\n");
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
    if (nargs < 2)
    {
        fprintf(stderr, "%s: too few arguments\n", args[0]);
        fprintf(stderr, "Try '%s --help' for more information\n",
                args[0]);
        return SYSERR;
    }

    printf("Hello %s, Welcome to the world of Xinu!!\n",args[1]);

    return OK;
}
