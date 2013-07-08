#include <types.h>
#include <lib.h>
#include <thread.h>
#include <waitpid.h>
#include <pidHandler.h> 
extern struct process* procTable[300];

struct lock* waitlock;
struct cv* waitlist;
pid_t sys_waitpid(pid_t pid, int* status, int options)
{
	lock_acquire(waitlist);
	int i, stat;
	*status = stat;
	options = 0;
	struct process* curProc = kmalloc(sizeof(struct process));
		
	for(i = 0 ; i < 300 ; i++){
		procTable[i] = curProc;
		if(pid == curProc->ppid){
			if(curProc->t == NULL){
				kprintf("Process doesn't exist");
				return -1;
			}
			while(!(curProc->exited)){
				cv_wait(waitlist, waitlock);
			}
			assert(curProc->exited == 1);
			cv_broadcast(waitlist, waitlock);
			return stat;
		}
		else{//pid doesn't exist
			kprintf("Pid waited on doesn't exist\n");
			return -1;
		}
	}
	lock_release(waitlist);
	kfree(curProc);
 	return 0;
}

