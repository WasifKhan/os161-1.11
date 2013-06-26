#include <types.h>
#include <lib.h>
#include <exit.h>
#include <thread.h>

void sys__exit(int exitcode) {
	// handle exit status later
	thread_exit();
}
