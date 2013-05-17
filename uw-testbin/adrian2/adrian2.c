#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

// Developed by Adrian Duong in F12

/*
 * This program will test IO from forked processes.
 * We will write to a file, and then create a number
 * of child processes to read the file.
 */

// Number of child processes to read from file.
#define NUM_CHILDREN 10

static char buffer[100];

int main(int argc, char **argv) { 
  (void) argc;
  (void) argv;

  // Create a file to use for testing.
  char filename[] = "a2test.in";
  int fileid;

  // Get the ID of a file to write. If it does not exist, create it.
  // If the file exists already, truncate its length to 0.
  fileid = open(filename, O_WRONLY|O_CREAT|O_TRUNC);

  int len;
  int i;
  for (i=0; i<NUM_CHILDREN; i++) {
    // Write increasing numbers to file.
    snprintf(buffer, sizeof(buffer), "%-5d", i);
    len = write(fileid, buffer, strlen(buffer));

    // Exit if we can't write to the file.
    if (len<0) {
      printf("FAILURE\n");
      err(1, "Could not write to file %s", filename);
    }
  }
  close(fileid);

  // Read from the file we just wrote to.
  fileid = open(filename, O_RDONLY);

  char inbuffer[6];

  int pid;
  int exit_code;
  for (i=0; i<NUM_CHILDREN; i++) {
    pid = fork();
    if (pid == 0) {
      len = read(fileid, inbuffer, sizeof(inbuffer)-1);
      inbuffer[5] = 0;

      printf("%s\n", inbuffer);
      close(fileid);
      exit(0);
    } else {
      waitpid(pid, &exit_code, 0);
    }
  }

  printf("SUCCESS\n");
  return 0;
}
