#include <prodcons.h>

uint future_cons(future *fut) {
  int i, status;
  status = future_get(fut, &i);
  if (status == SYSERR) {
    printf("future_get failed\n");
    return -1;
  }
  printf("consumed %d\n", i);
  return OK;
}

