#include <queue.h>
#include <types.h>
#include <lib.h>

struct PID_handler {
   pid_t pidCounter;
   struct queue * pids;
   struct array * flags;
};

pid_t getPID(struct PID_handler * handler);

void returnPID (pid_t id, struct PID_handler * handler);

struct PID_handler * createHandler ();
