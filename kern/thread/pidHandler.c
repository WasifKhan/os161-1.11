#include <pidHandler.h>
#include <queue.h>
#include <types.h>
#include <lib.h>

/*
struct process * createProcesses (struct PID_handler * handler) {
   int i;
   for (i = 1; i < 300; i++) {
      struct process * p = kmalloc(sizeof(struct createProcess));
      p->mypid = (pid_t)i;
      p->exitCode = -1;
      p->t = NULL;
      array_setguy(handler->flags, i, p);
      q_addtail(handler->pids, i);
   }
}
*/



pid_t getPID (struct PID_handler * handler) {
      pid_t pid = q_remhead(handler->pids);         
      array_setguy (handler->flags, (int)pid, 1);
      return pid;
}

void returnPID (pid_t pid, struct PID_handler * handler) { //, int exitcode) {
   q_addtail(handler->pids,pid);
   array_setguy(handler->flags,(int)pid, 2); //exitcode);
   return;
}

struct PID_handler * createHandler () {
   struct PID_handler * handler = kmalloc(sizeof(struct PID_handler));
   handler->flags = array_create();
   array_preallocate (handler->flags, 300);
   array_setsize(handler->flags, 300);
   handler->pids = q_create(1);  // initiate to 1, grows when necessary
   int i;
   for (i = 0; i< 300; i++) {
      array_setguy(handler->flags, i, 0);
      if (i != 0) {
         q_addtail(handler->pids, i);
      }
   }
   return handler;
}
