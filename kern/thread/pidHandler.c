#include <pidHandler.h>
#include <queue.h>
#include <types.h>
#include <lib.h>


struct process * createPT () {
   int i;
 struct process * p = kmalloc(300*sizeof(struct process));  
 for (i = 0; i < 300; i++) {
      p->ppid = 0;
      p->exited = 0;
      p->exitCode = -1;
      p->t = NULL;
   }
}


pid_t getPID (struct PID_handler * handler) {
      pid_t pid = (pid_t)q_remhead(handler->pids);         
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
   q_preallocate(handler->pids, 299);
   int i;
   for (i = 0; i< 300; i++) {
     if (i == 0) {
      array_setguy(handler->flags, i, -1);
     } else {
      array_setguy(handler->flags, i, 0);
      }
   }

   for (i = 1; i < 50; i++) {
      q_addtail(handler->pids, i);
   }
     
     
     /*
      if (i == 0) {
         array_setguy(handler->flags, i, -1);
      } else {
         array_setguy(handler->flags, i, 0);
         q_addtail(handler->pids, i);
      }
   }
   */
   return handler;
}
