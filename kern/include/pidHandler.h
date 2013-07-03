#include <queue.h>
#include <types.h>
#include <lib.h>

struct PID_handler {
   int pidCounter;
   struct queue * recycled;
   struct array * flags;
};

int getPID(struct PID_handler * handler);

void returnPID (int id, struct PID_handler * handler);

struct PID_handler * createHandler ();
