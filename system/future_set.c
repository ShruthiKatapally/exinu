#include <kernel.h>
#include <future.h>

syscall future_set(future *f, int value) 
{
  if(f->state == FUTURE_EMPTY || f->state == FUTURE_WAITING)
{
    switch(f->flag)
 {
      case FUTURE_EXCLUSIVE:
        f->value = value;
        f->state = FUTURE_VALID;
        future_signal(f);
        break;

      case FUTURE_SHARED:
	f->value = value;
        f->state = FUTURE_VALID;
        future_signal(f);
        break;

      case FUTURE_QUEUE:
        if(isempty(f->get_queue)) {
          //if the get queue is empty, wait on this future
	  enqueue(gettid(), f->set_queue);
          future_wait(f);
	  f->value = value;
          f->state = FUTURE_VALID;
          future_signal(f);
        }
        else {
          //else set the value and signal future
          f->value = value;
          f->state = FUTURE_VALID;
          future_signal(f);  
        }
        break;

      default:
        return SYSERR;
        break;
    }
  }
  else if(f->state == FUTURE_VALID){
    switch(f->flag)
 {
      case FUTURE_EXCLUSIVE:
        return SYSERR;
        break;

      case FUTURE_SHARED:
        return SYSERR;
        break;

      case FUTURE_QUEUE:
        return SYSERR;
        break;

      default:
        return SYSERR;
        break;
    }
  }
  else {
    return SYSERR;
  }
  return OK;
}
