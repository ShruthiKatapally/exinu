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
       //kprintf("future produced\n");
        future_signal(f);
        break;

      case FUTURE_SHARED:
	f->value = value;
        f->state = FUTURE_VALID;
       //kprintf("future produced in shared mode \n");
        future_signal(f);
        break;

      case FUTURE_QUEUE:
        if(isempty(f->get_queue)) {
          //if the get queue is empty, wait on this future
	  //kprintf("reached queue empty in set. q# %d\n", f->get_queue);
	  enqueue(gettid(), f->set_queue);
          future_wait(f);
	  f->value = value;
          f->state = FUTURE_VALID;
          //kprintf("future produced\n");
          future_signal(f);
        }
        else {
          //else set the value and signal future
	  //kprintf("reached queue not empty in set. q# %d\n",f->get_queue);
          f->value = value;
          f->state = FUTURE_VALID;
          //kprintf("future produced\n");
          future_signal(f);  
        }
        break;

      default:
       //kprintf("invalid future flag\n");
        return SYSERR;
        break;
    }
  }
  else if(f->state == FUTURE_VALID){
    switch(f->flag)
 {
      case FUTURE_EXCLUSIVE:
        //kprintf("future is not empty\n");
        return SYSERR;
        break;

      case FUTURE_SHARED:
        //kprintf("future is not empty\n");
        return SYSERR;
        break;

      case FUTURE_QUEUE:
        //kprintf("future is not empty, tid = %d \n",gettid());
        return SYSERR;
        //future is yet to be consumed, wait on this future
	/*enqueue(gettid(), f->set_queue);
        future_wait(f);
	f->value = value;
        f->state = FUTURE_VALID;
        //kprintf("future produced\n");
        future_signal(f);*/
        break;

      default:
       //kprintf("invalid future flag\n");
        return SYSERR;
        break;
    }
  }
  else {
   //kprintf("invalid future state\n");
    return SYSERR;
  }
  return OK;
}
