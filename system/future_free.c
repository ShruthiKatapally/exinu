#include <kernel.h>
#include <future.h>

syscall future_free(future* f){

  return memfree(f,sizeof(future));

}
