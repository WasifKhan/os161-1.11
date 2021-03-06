/*
 * forktest - test fork().
 *
 * This should work correctly when fork is implemented.
 *
 * It should also continue to work after subsequent assignments, most
 * notably after implementing the virtual memory system.
 */

#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <err.h>

/*
 * This is used by all processes, to try to help make sure all
 * processes have a distinct address space.
 */
static volatile int mypid;

/*
 * Helper function for fork that prints a warning on error.
 */
static
int
dofork(void)
{
	int pid;
	pid = fork();
	if (pid < 0) {
		warn("fork");
	}
	return pid;
}

/*
 * Check to make sure each process has its own address space. Write
 * the pid into the data segment and read it back repeatedly, making
 * sure it's correct every time.
 */
static
void
check(void)
{
	int i;

	mypid = getpid();
	/* Make sure each fork has its own address space. */
	for (i=0; i<800; i++) {
		volatile int seenpid;
		seenpid = mypid;
		if (seenpid != getpid()) {
			errx(1, "pid mismatch (%d, should be %d) "
			     "- your vm is broken!", 
			     seenpid, getpid());
		}
	}
}

/*
 * Wait for a child process.
 *
 * This assumes dowait is called the same number of times as dofork
 * and passed its results in reverse order. Any forks that fail send
 * us -1 and are ignored. The first 0 we see indicates the fork that
 * generated the current process; that means it's time to exit. Only
 * the parent of all the processes returns from the chain of dowaits.
 */
static
void
dowait(int nowait, int pid)
{
/*	int x;

	if (pid<0) {
		// fork in question failed; just return 
		printf("pid is < 0, woops \n");
      return;
	}
*/
	if (pid==0) {
      printf("exiting!\n");
		/* in the fork in question we were the child; exit */
	//	exit(0);
	}
/*
	if (!nowait) {
      printf("shouldn't get here\n");
		if (waitpid(pid, &x, 0)<0) {
			warn("waitpid");
		}
		else if (x!=0) {
			warnx("pid %d: exit %d", pid, x);
		}
	}
   */
}

/*
 * Actually run the test.
 */
static
void
test(int nowait)
{
	int pid0, pid1, pid2, pid3;

	/*
	 * Caution: This generates processes geometrically.
	 *
	 * It is unrolled to encourage gcc to registerize the pids,
	 * to prevent wait/exit problems if fork corrupts memory.
	 */
	pid0 = dofork();
   putchar('0');
	check();
	pid1 = dofork();
	putchar('1');
	check();
	pid2 = dofork();
	putchar('2');
	check();
	pid3 = dofork();
   putchar('3');
	check();
   /*
	 * These must be called in reverse order to avoid waiting
	 * improperly.
	*/
   /*
   pid0 = dofork();
   putchar('0');
   pid1 = dofork();
   putchar('1');
   */
   /*if (pid0 != 0) {
      pid1 = dofork();
      putchar('2');
   }
   if (pid0 == 0) {
      pid2 = dofork();
      putchar ('3');
   }*/
   dowait(nowait, pid3);
	dowait(nowait, pid2);
	dowait(nowait, pid1);
	dowait(nowait, pid0);
   putchar('\n');
}

int
main(int argc, char *argv[])
{
   //int nowait=1;


//   int nowait=0;
//>>>>>>> Stashed changes
/*	if (argc==2 && !strcmp(argv[1], "-w")) {
		nowait=1;
	}
	else if (argc!=1 && argc!=0) {
		warnx("usage: forktest [-w]");
		return 1;
	}
*/
	/*
	warnx("Starting.");

	test(nowait);

	warnx("Complete.");
*/
/*
	int p1,p2,p3;
	printf("\n0\n");
	p1 = fork();
	printf("\n1\n");
	p2 = fork();
	printf("\n2\n");
	p3 = fork();
	printf("\n3\n");
*/
	return 0;
}
