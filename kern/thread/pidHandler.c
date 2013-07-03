#include <pidHandler.h>
#include <queue.h>
#include <types.h>
#include <lib.h>

int getPID (struct PID_handler * handler) {
   if (q_empty(handler->recycled) == 1) {
      handler->pidCounter++;
      return handler->pidCounter;
   } else {
      int pid = (int)q_remhead(handler);         
      return pid;
   }
}

void returnPID (int pid, struct PID_handler * handler) {
   q_addtail(handler->recycled, pid);
   return;
}

struct PID_handler * createHandler () {
   struct PID_handler * handler = kmalloc(sizeof(struct PID_handler));
   
   handler->flags = array_create();
   array_preallocate (handler->flags, 200);
   array_setsize(handler->flags, 200);

   int i=0;
   for (i = 0; i < 200; i++) {
      array_setguy (handler->flags, i, 0);
   }

   handler->pidCounter = 1;
   handler->recycled = q_create(1); // initialize to 1, grows when necessary
   return handler;
}


