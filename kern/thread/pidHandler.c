#include <pidHandler.h>
#include <queue.h>
#include <types.h>
#include <lib.h>

pid_t getPID (struct PID_handler * handler) {
      pid_t pid = (int)q_remhead(handler->pids);         
      array_setguy (handler->flags, (int)pid, 1);
      return pid;
}

void returnPID (pid_t pid, struct PID_handler * handler) {
   q_addtail(handler->pids,pid);
   array_setguy(handler->flags,(int)pid, 2);
   return;
}

struct PID_handler * createHandler () {
   struct PID_handler * handler = kmalloc(sizeof(struct PID_handler));
   
   handler->flags = array_create();
   array_preallocate (handler->flags, 200);
   array_setsize(handler->flags, 200);
   handler->pids = q_create(1);  // initiate to 1, grows when necessary
   
   int i=0;
   for (i = 0; i < 200; i++) {
      array_setguy (handler->flags, i, 0);
      q_addtail(handler->pids, i);
   }
   return handler;
}
