/**
 * @file uartWrite.c
 * @provides uartWrite.
 *
 * $Id: uartWrite.c 2102 2009-10-26 20:36:13Z brylow $
 */
/* Embedded Xinu, Copyright (C) 2009.  All rights reserved. 
   copied from the ns16550 stuff
 */

#include <stddef.h>
#include <uart.h>
#include "uart.h"
#include "fluke-uart.h"
#include <device.h>
#include <interrupt.h>

/**
 * Write a buffer to the UART.
 *
 * @param devptr  pointer to UART device
 * @param buf   buffer of characters to write
 * @param len   number of characters to write from the buffer
 */
devcall uartWrite(device *devptr, void *buf, uint len)
{
    irqmask im;
    int count = 0;
    struct uart *uartptr;
    volatile struct uart_csreg *regptr;
    char *buffer = buf;

    uartptr = &uarttab[devptr->minor];
    regptr = (struct uart_csreg *)uartptr->csr;
    if (NULL == regptr)
    {
        return SYSERR;
    }

    im = disable();

    /* If in non-blocking mode, ensure there is enough space for the entire
     * write request */
    if ((uartptr->oflags & UART_OFLAG_NOBLOCK)
        && (semcount(uartptr->osema) < len))
    {
        restore(im);
        return SYSERR;
    }

    /* Put each character from the buffer into the output buffer for the
     * lower half */
    while (count < len)
    {
        /* If in non-blocking mode, ensure there is another byte of space
         * for output */
        if ((uartptr->oflags & UART_OFLAG_NOBLOCK)
           && (semcount(uartptr->osema) < 1))
        {
            break;
        }
      
        /* If the UART transmitter hardware is idle, write directly to the
         * hardware */
        if (uartptr->oidle)
        {
	  uartptr->oidle = FALSE; // uncomment when we have an ISR defined for this... fo sho
	  regptr->thr = *buffer++;
	  count++;
	  uartptr->cout++;
        }
        /* Wait for space and place character in the output buffer for the
         * lower half; Preserve the circular buffer */
        else
        {
	    wait(uartptr->osema); 
            uartptr->out[(uartptr->ostart +
                          uartptr->ocount) % UART_OBLEN] = *buffer++;
            count++;
            uartptr->ocount++;
        }
    }

    restore(im);
    return count;
    
}
