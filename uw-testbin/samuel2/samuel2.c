#include <unistd.h>
#include <err.h>
#include <stdio.h>
#include <string.h>

// developed by Samuel Yuan in F12

/*
 * my-a2-test
 * Calls open, read, write, close, fork, waitpid, and getpid
 * Opens two files, closes the first, and forks two threads. Main thread and two
 * children write concurrently to the second file. Tests if file descriptors are
 * copied correctly to child processes, if closing in a child affects the
 * parent, and if write is atomic.
 */

#define MAXPROCS  6
#define MAIN_WRITES 50
#define CHILD_WRITES 50
static int pids[MAXPROCS], npids = 0;
static char buffer[100];

static
int
forkwrite(int fd)
{
	int len;
	int i;
	int pid = fork();
	switch (pid) {
		case -1:
			err(1, "fork");
			break;
		case 0:
			/* child */
			snprintf(buffer, sizeof(buffer), "Child (%d)\n", getpid());
			for (i = 0; i < CHILD_WRITES; i++) {
				len = write(fd, buffer, strlen(buffer));
				if (len < 0) {
					err(1, "%d: write", getpid());
				}
			}
			close(fd);
			_exit(0);
		default:
			/* parent */
			pids[npids++] = pid;
			return pid;
	}
	return 0;
}

static
void
waitall(void)
{
	int i, status;
	for (i=0; i<npids; i++) {
		// printf("Waiting for %d\n", pids[i]);
		if (waitpid(pids[i], &status, 0) < 0) {
			warn("waitpid for %d", pids[i]);
		}
		else if (status != 0) {
			warnx("pid %d: exit %d", pids[i], status);
		}
	}
}

int
main() {
	char cstr1[50], cstr2[50];
   	const char *mstr1 = "Main program during write\n",
		  *mstr2 = "Main program after close\n";
	int a, b;
	int len;
	int i;

	a = open("a", O_WRONLY|O_CREAT|O_TRUNC);
	b = open("b", O_WRONLY|O_CREAT|O_TRUNC);
	close(a);

	a = forkwrite(b);
	snprintf(cstr1, 50, "Child (%d)\n", a);
	a = forkwrite(b);
	snprintf(cstr2, 50, "Child (%d)\n", a);

	for (i = 0; i < MAIN_WRITES; i++) {
		len = write(b, mstr1, strlen(mstr1));
		if (len < 0) {
			err(1, "main: write");
		}
	}

	// Should still be able to write to files closed in children
	waitall();
	write(b, mstr2, strlen(mstr2));

	close(b);

	// Check atomicity of write
	int linelen = 0;
	b = open("b", O_RDONLY);
	// Should really read more than 1 byte at a time
	while ((len = read(b, buffer + linelen, 1)) > 0) {
		linelen++;
		if (buffer[linelen-1] == '\n') {
			buffer[linelen] = '\0';
			if (strcmp(buffer, cstr1) && strcmp(buffer, cstr2) &&
					strcmp(buffer, mstr1) && strcmp(buffer, mstr2)) {
				printf("FAILURE\n");
				close(b);
				return 0;
			}
			linelen = 0;
		}
	}
	printf("SUCCESS\n");
	close(b);

	return 0;
}
