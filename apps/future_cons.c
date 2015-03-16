#include <prodcons.h>

uint future_cons(future *fut) {
  //kprintf("tid = %d\n",gettid());
  int i, status;
  status = future_get(fut, &i);
  if (status == SYSERR) {
    printf("future_get failed\n");
    return -1;
  }
  kprintf("consumed %d\n", i);

  if(fut->flag == FUTURE_EXCLUSIVE){
    if(future_free(fut)==SYSERR){
      printf("future_free failed\n");
    } 
  }
/*  if(fut->flag == FUTURE_SHARED) {
    if (isempty(fut->get_queue)) { 
      if(future_free(fut)==SYSERR) {
        printf("future_free failed\n");
      }
      kprintf("future_free success\n");
    }
  }
  if(fut->flag == FUTURE_QUEUE){
    if (isempty(fut->get_queue) && isempty(fut->set_queue)) {
      if(future_free(fut)==SYSERR) {
        printf("future_free failed\n");
      } 
    }
  }
*/ 
  return OK;
}

