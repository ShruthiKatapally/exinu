#include <kernel.h>
#include <future.h>

future* future_alloc(int future_flag){
  if(future_flag!=FUTURE_EXCLUSIVE){
    printf("future_alloc: invalid flag\n");
    return NULL;
  }

  future *fut;
  fut = (future *) memget(sizeof(future));

  if (SYSERR == (uint)fut){
    printf("future_alloc: insufficient memory\n");
    return NULL;
  }
  else
  {
    fut->value = 0;
    fut->state = FUTURE_EMPTY;
    fut->flag = FUTURE_EXCLUSIVE;
    fut->tid = -1;
    return fut;
  }  
}
