#include <types.h>
#include <lib.h>
#include <exit.h>
#include <thread.h>
#include <curthread.h>
#include <pidHandler.h>

extern struct PID_handler *pids;
extern struct process* procTable[300];

void sys__exit(int exitcode) {

	pid_t pid = curthread->pid;
	
	array_setguy(pids->flags, (int)pid, exitcode);
	int i;
	for(i = 0; i < 300 ; i++){
		if(procTable[i]->ppid == pid){
			procTable[i]->exited = 1;
			procTable[i]->exitCode = exitcode;
		//	kprintf("proc #%d exited\n", procTable[i]->ppid);
			break; 
		}
	}

	thread_exit();
}
