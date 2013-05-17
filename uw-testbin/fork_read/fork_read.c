/*
 * the_road.c
 *
 * 	Run a few processes that will attempt to read/write from a file.
 *      The file they attempt to read/write from is "frost". This should be a copy of the provided file.
 *
 * This test should itself run correctly when the basic system calls are complete in addition to some I/O.
 */

/*
 * Purpose: Test that reading from the same file descriptor in a child and
 * parent simultaneously does not cause problems. Additionally tests that the
 * information read is in-line with what the man pages specify.
 * Required Input Files: frost (included in uw-testbin/fork_read/ ).
 * Logical breakdown: Open file and read from it to verify read works. Fork,
 * and perform a read in both parent and child on the already-opened file.
 * Check which line child read, close file in child and report the read line to
 * parent using waitpid. Check which line parent read, and make sure it is not
 * the same as what child read. Perform one final read in parent, close file
 * and end.
 * Expected Output: The program will always print which string was read by the
 * child and which was read by the parent (“Parent read first string”, “Child
 * read second string”). A successful run will show that parent read either
 * first or second, and that child read the one that parent did not read
 * (although the ordering in which this prints out is not important). Any
 * errors that occur will print an explicit message containing the word
 * “error”.
 * Possible Extensions: Re-implementing this test without waitpid may be
 * beneficial as then it will rely on less system calls (waitpid really isn’t
 * the functionality we want to test for, but it is necessary for it to work in
 * order for the sudents to receive marks currently).
 */

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <err.h>

#define READSIZE 5
static int child_pid;
static char read2buf[READSIZE + 1];

// Read in the next 
static int read_next(int fd)
{
	int ret_val, pid = fork();
	switch (pid) {
	    case -1: // Error and die
			err(1, "Child fork error");
	    case 0: // Child process, break out
			if ((ret_val = read(fd, read2buf, READSIZE)) < 0 || ret_val != READSIZE) {
				err(1, "Second read error (child)");
			}
			break;
	    default: // Parent process, increase wait pids
			if ((ret_val = read(fd, read2buf, READSIZE)) < 0 || ret_val != READSIZE) {
				err(1, "Second read error (parent)");
			}
			child_pid = pid;
			break;
	}
	read2buf[READSIZE] = '\0';
	return pid;
}

// Wait on all running pids before doing some magic comparison stuff
static void wait_on_results(int parent_ret_val)
{
	int child_ret_val;
	if (waitpid(child_pid, &child_ret_val, 0) < 0) {
		err(1, "Wait for child error");
	}

	// Make sure there is no parent error
	if (parent_ret_val == -1) {
		err(1, "Parent seek position after fork error");
	}
	
	// Make sure there is no child error
	if (child_ret_val == -1) {
		err(1, "Child seek position after fork error");
	}

	if (!(parent_ret_val == 1 && child_ret_val == 2) || (parent_ret_val == 2 && child_ret_val == 1)) {
		err(1, "Seek position persistence error (parent read %d, child read %d)", parent_ret_val, child_ret_val);
	}

	return;
}

// Main entry point for the program
int main()
{
	static char filename[6] = "frost"; // File to open
	static char first_read[READSIZE + 1] = "Two r"; // The first series of read characters
	static char second_read1[READSIZE + 1] = "oads "; // One of the next series of characters to read
	static char second_read2[READSIZE + 1] = "diver"; // One of the next series of characters to read
	static char last_read[READSIZE + 1] = "ged i"; // The last expected series of characters

	// NB: second_read and third_read arrive in non-deterministic order

	static char readbuf[READSIZE + 1]; // Buffer to read into
	int fd, pid, ret_val;

	// Open frost file
	if ((fd = open(filename, O_RDONLY)) < 0) {
		err(1, "%s: Open for read error", filename);
	}

	// Perform first read
	if ((ret_val = read(fd, readbuf, READSIZE)) < 0 || ret_val != READSIZE) {
		err(1, "First read error");
	}
	readbuf[READSIZE] = '\0';

	// Make sure read works to begin with
	if (strcmp(readbuf, first_read)) {
		err(1, "First read mismatch error");
	}

	// Get next two reads and recide which one has been read
	pid = read_next(fd);
	ret_val = -1;
	if (!strcmp(read2buf, second_read1)) {
		ret_val = 1;
		if (pid == 0) {
			printf("Child read first string\n");
		}
		else {
			printf("Parent read first string\n");
		}
	}
	else if (!strcmp(read2buf, second_read2)) {
		ret_val = 2;
		if (pid == 0) {
			printf("Child read second string\n");
		}
		else {
			printf("Parent read second string\n");
		}
	}

	// Return from child immediately
	if (pid == 0) {
		// Close the fd in the child
		if (close(fd)) {
			err(1, "Child close error");
		}
		return ret_val;
	}

	// Check which results were which
	wait_on_results(ret_val);

	// Perform last read
	if ((ret_val = read(fd, readbuf, READSIZE)) < 0 || ret_val != READSIZE) {
		err(1, "Last read error");
	}
	readbuf[READSIZE] = '\0';

	// Make sure read works to begin with
	if (strcmp(readbuf, last_read)) {
		err(1, "Last read mismatch error");
	}
	
	// Close the fd in the parent
	if (close(fd)) {
		err(1, "Parent close error");
	}

	return 0;
}

