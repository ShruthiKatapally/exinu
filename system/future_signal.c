/**
 * @file future_signal.c
 *
 */

#include <thread.h>
#include <future.h>
#include <queue.h>

/**
 * @ingroup future
 *
 * Signal a future, releasing up to one waiting thread.
 *
 * signal() may reschedule the currently running thread.  As a result, signal()
 * should not be called from non-reentrant interrupt handlers unless ::resdefer
 * is set to a positive value at the start of the interrupt handler.
 *
 * @param fut
 *      future to signal.
 *
 * @return
 *      ::OK on success, ::SYSERR on failure.  This function can only fail if @p
  */
syscall future_signal(future *fut)
{
    irqmask im;

    im = disable();

    if (fut->state == FUTURE_VALID)
    {
	    switch(fut->flag) {
	      case FUTURE_EXCLUSIVE:
		ready(fut->tid, RESCHED_YES);
		break;

	      case FUTURE_SHARED:
		while (!isempty(fut->get_queue))
		{
		ready(dequeue(fut->get_queue), RESCHED_YES);
		}
		break;

	      case FUTURE_QUEUE:
                ready(dequeue(fut->get_queue), RESCHED_YES);
		break;

	      default:
		return SYSERR;
		break;
	    }   
    }
    else if (fut->state == FUTURE_EMPTY)
    {
	    switch(fut->flag) {
	      case FUTURE_EXCLUSIVE:
		break;

	      case FUTURE_SHARED:
		break;

	      case FUTURE_QUEUE:
                ready(dequeue(fut->set_queue), RESCHED_YES);
		break;

	      default:
		return SYSERR;
		break;
	    }   
	
    }
    restore(im);
    return OK;
}
