#include <kernel.h>
#include <future.h>

syscall future_set(future *f, int value) {
  if(f->state == FUTURE_EMPTY || f->state == FUTURE_WAITING){
    switch(f->flag) {
      case FUTURE_EXCLUSIVE:
        f->value = value;
        f->state = FUTURE_VALID;
        //kprintf("future produced\n");
        future_signal(f);
        break;

      case FUTURE_SHARED:
        break;

      case FUTURE_QUEUE:
        if(isempty(f->get_queue)) {
          //if the get queue is empty, wait on this future
          future_wait(f);
        }
        else {
          //else set the value and signal future
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
    //kprintf("future is not empty\n");
    return SYSERR;
  }
  else {
    //kprintf("invalid future state\n");
    return SYSERR;
  }
  return OK;
}
