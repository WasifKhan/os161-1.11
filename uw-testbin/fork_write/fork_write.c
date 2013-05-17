/*
 * the_road.c
 *
 * 	Run a few processes that will attempt to read/write from a file.
 *      The file they attempt to read/write from is "frost". This should be a copy of the provided file.
 *
 * This test should itself run correctly when the basic system calls are complete in addition to some I/O.
 */

/*
 * Purpose: Test that reading from the same file descriptor in a parent after
 * writing to it in a forked child does not cause problems and returns the
 * written values as expected.
 * Required Input Files: N/A (outputs a file named “new_frost”)
 * Logical breakdown: Open / create / truncate file, and then fork. In child,
 * write to the file (using the already-received file descriptor from the
 * parent), and close it. In parent, waitpid on child. Then, close the file,
 * re-open it, and read what was written by the child. Close the file again and
 * exit.
 * Expected Output: The program will only ever print in case of errors. A
 * successful test will print nothing.
 * Possible Extensions: Write to the file and then read EOF in parent before
 * closing and then testing the read on a fresh open (this seemed fairly
 * pedantic, so I didn’t include it).
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <err.h>

#define READSIZE 5
static int child_pid;

// Read in the next 
static int write_next(int fd)
{
	static char writebuf[37] = "And that has made all the difference"; // 36 chars + \0

	int ret_val, pid = fork();
	switch (pid) {
	    case -1: // Error and die
			err(1, "Child fork error");
	    case 0: // Child process, break out
			if ((ret_val = write(fd, writebuf, READSIZE)) < 0 || ret_val != READSIZE) {
				err(1, "Child write error");
			}
			break;
	    default: // Parent process, increase wait pids
			child_pid = pid;
			break;
	}
	return pid;
}

// Wait on all running pids before doing some magic comparison stuff
static void wait_on_child()
{
	int child_ret_val;
	if (waitpid(child_pid, &child_ret_val, 0) < 0) {
		err(1, "Wait for child error");
	}
	return;
}

// Main entry point for the program
int main()
{
	static char filename[10] = "new_frost"; // File to open
	static char first_read[READSIZE + 1] = "And t"; // The last expected series of characters
	static char readbuf[READSIZE + 1]; // Buffer to read into
	int fd, pid, ret_val;

	// Open frost file
	if ((fd = open(filename, O_RDWR | O_CREAT | O_TRUNC)) < 0) {
		err(1, "%s: First open error", filename);
	}

	pid = write_next(fd); // Get the child to write to the file
	// Return from child immediately
	if (pid == 0) {
		// Close the fd in the child
		if (close(fd)) {
			err(1, "Child close error");
		}
		return ret_val;
	}

	wait_on_child(); // Wait on the child to finish

	// Close the fd in the parent
	if (close(fd)) {
		err(1, "Parent first close error");
	}

	// Open frost file
	if ((fd = open(filename, O_RDONLY)) < 0) {
		err(1, "%s: Second open error", filename);
	}

	// Perform consistency read
	if ((ret_val = read(fd, readbuf, READSIZE)) < 0 || ret_val != READSIZE) {
		err(1, "Last read error");
	}
	readbuf[READSIZE] = '\0';

	// Make sure what we expected to be written then read was actually written and then read
	if (strcmp(readbuf, first_read)) {
		err(1, "Read/write consistency error");
	}
	
	// Close the fd in the parent
	if (close(fd)) {
		err(1, "Parent second close error");
	}

	return 0;
}

