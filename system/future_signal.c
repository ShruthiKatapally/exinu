/**
 * @file future_signal.c
 *
 */

#include <thread.h>
#include <future.h>

/**
 * @ingroup future
 *
 * Signal a future, releasing up to one waiting thread.
 *
 * signal() may reschedule the currently running thread.  As a result, signal()
 * should not be called from non-reentrant interrupt handlers unless ::resdefer
 * is set to a positive value at the start of the interrupt handler.
 *
 * @param sem
 *      Semaphore to signal.
 *
 * @return
 *      ::OK on success, ::SYSERR on failure.  This function can only fail if @p
 *      sem did not specify a valid semaphore.
 */
syscall future_signal(future *fut)
{
    //register struct sement *semptr;
    irqmask im;

    im = disable();
    /*if (isbadsem(sem))
    {
        restore(im);
        return SYSERR;
    }*/
    //semptr = &semtab[sem];
    if (fut->state == FUTURE_VALID)
    {
        ready(fut->tid, RESCHED_YES);
    }
    restore(im);
    return OK;
}
