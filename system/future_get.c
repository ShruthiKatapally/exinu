#include <kernel.h>
#include <future.h>

syscall future_get(future *f, int *value) {
  if(f->state == FUTURE_EMPTY){
    switch(f->flag) {
      case FUTURE_EXCLUSIVE:
        //f->tid = gettid();
        //kprintf("future empty\n");
        f->state = FUTURE_WAITING;
        future_wait(f);
        *value = f->value;
        f->state = FUTURE_EMPTY;
        //kprintf("future consumed\n");
        break;

      case FUTURE_SHARED:
        break;

      case FUTURE_QUEUE:
        if(isempty(f->set_queue)) {
          //if the get queue is empty, wait on this future
          future_wait(f);
        }
        else {
          //else signal future
          future_signal(f);  
        }
        break;

      default:
        //kprintf("invalid future flag\n");
        return SYSERR;
        break;
    }
  } 
  else if(f->state == FUTURE_WAITING){
    switch(f->flag) {
      case FUTURE_EXCLUSIVE:
        //kprintf("future waiting\n");
        return SYSERR;
        break;

      case FUTURE_SHARED:
        break;

      case FUTURE_QUEUE:
        break;

      default:
        //kprintf("invalid future flag\n");
        return SYSERR;
        break;
    }
  } 
  else if(f->state == FUTURE_VALID){
    switch(f->flag) {
      case FUTURE_EXCLUSIVE:
        *value = f->value;
        f->state = FUTURE_EMPTY;
        //kprintf("future valid\n");
        break;

      case FUTURE_SHARED:
        break;

      case FUTURE_QUEUE:
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
