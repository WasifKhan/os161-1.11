#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <err.h>

#define PAGE_SIZE       (4096)
#define PAGES           (1024)  /* 4 MB worth of data */
#define SIZE            (PAGE_SIZE * PAGES / sizeof(int))

// Developed by Adrian Duong in F12

/*
 * This program tests that memory is freed correctly after a process exits.
 * We test the memory management system by having one process exhaust the memory
 * by allocating 4 MB of ram. Then, when this process exists, an other one
 * also exhausts the memory and checks that information is not lost.
 */

static unsigned int array[SIZE];

int main() { 
  unsigned int i;
  unsigned int index;
  int exit_code;

  int pid = fork();
  if (pid == 0) {
		for (i=0; i<PAGES; i++) {
			index = i * PAGE_SIZE  / sizeof(int);
			array[index] = i;
		}
  	for (i=0; i<PAGES; i++) {
  		index = i * PAGE_SIZE  / sizeof(int);
  		if (array[index] != i) {
  			exit(1);
  		}
  	}
    exit(0);
  } else {
    waitpid(pid, &exit_code, 0);
    if (exit_code != 0) {
      printf("FAILURE\n");
      exit(1);
    }
		for (i=0; i<PAGES; i++) {
			index = i * PAGE_SIZE  / sizeof(int);
			array[index] = i + 1;
		}
  	for (i=0; i<PAGES; i++) {
  		index = i * PAGE_SIZE  / sizeof(int);
  		if (array[index] != i + 1) {
        printf("FAILURE\n");
  			exit(1);
  		}
  	}
  }
  printf("SUCCESS\n");
  return 0;
}
