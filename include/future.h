#ifndef _FUTURE_H_
#define _FUTURE_H_

#include <queue.h>
#include <kernel.h>
 
/* define states */
#define FUTURE_EMPTY	  0
#define FUTURE_WAITING 	  1         
#define FUTURE_VALID 	  2         

/* modes of operation for future*/
#define FUTURE_EXCLUSIVE  1
#define FUTURE_SHARED	  2
#define FUTURE_QUEUE	  3	

typedef struct futent
{
   int value;		
   int flag;		
   int state;         	
   tid_typ tid;
   qid_typ set_queue;
   qid_typ get_queue;
} future;

/* Interface for system call */
future* future_alloc(int future_flags);
syscall future_free(future*);
syscall future_wait(future *fut);
syscall future_signal(future *fut);
syscall future_get(future*, int*);
syscall future_set(future*, int);
 
#endif /* _FUTURE_H_ */

