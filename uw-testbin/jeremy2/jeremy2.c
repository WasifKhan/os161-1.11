// Developed by Jeremy Roman in F12

/*
 * This test checks the behaviour of file descriptors after fork.
 * The offset into a file should be preserved by fork(), and it
 * it should be shared thereafter.
 *
 * To test this, the phrase "I think Albert Einstein is a great
 * physicist." is written to a (truncated/created) file. Then
 * it is rewound (by re-opening it), at which point the process
 * skips over the phrase "I think " (ensuring that it is correct),
 * then the process forks.
 *
 * The child first writes "Stephen", changing the text into
 * into "I think StephenEinstein is a great physicist.", and the
 * parent, after waiting for the child (and ensuring it was
 * successful), corrects the word "Einstein" to "Hawking". This
 * works because the parent's seek position has also advanced.
 *
 * Incorrect implementations will produce some other string in the
 * test file, which will not match the expected final message.
 *
 * This tests the following syscalls:
 *   _exit, fork, waitpid, open, read, write, close
 * More critically, it tests the interactions among them.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

const char filename[] = "/my-a2-test.out";
const char initial_message[] = "I think Albert Einstein is a great physicist.\n";
const char skip[] = "I think ";
const char child_message[] = "Stephen";
const char parent_message[] = " Hawking";
const char expected_message[] = "I think Stephen Hawking is a great physicist.\n";

void ensure(int condition) {
  if (!condition) {
    printf("FAILURE\n");
    exit(1);
  }
}

void checked_write(int fd, const char *buf) {
  int result, len = strlen(buf);
  do {
    result = write(fd, buf, len);
    ensure(result >= 0 && result <= len);
    buf += result;
    len -= result;
  } while (len > 0);
}

void checked_read(int fd, const char *buf) {
  int result;
  char c;
  while (*buf != '\0') {
    result = read(fd, &c, 1);
    ensure(result > 0);
    ensure(c == *buf);
    buf++;
  }
}

int main(int argc, char **argv) {
  int fd, status;
  pid_t child;
  char c;
  
  // Open the file. We must get a valid FD that isn't already taken.
  fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC);
  ensure(fd > 2);

  // Write the initial contents to the file.
  checked_write(fd, initial_message);

  // Close the file.
  ensure(close(fd) == 0);

  // Re-open the file (seek position should reset).
  fd = open(filename, O_RDWR);
  ensure(fd > 2);

  // Skip the first part of the message.
  // Ideally, we would use lseek for this.
  checked_read(fd, skip);

  // Fork. Let the child write its piece.
  child = fork();
  ensure(child >= 0);
  if (child == 0) {
    checked_write(fd, child_message);
    ensure(close(fd) == 0);
    exit(0);
  }

  // Wait on the child.
  ensure(waitpid(child, &status, 0) == child);
  ensure(status == 0);

  // Have the parent write its piece.
  checked_write(fd, parent_message);
  ensure(close(fd) == 0);

  // Now read the message back and ensure it matches the expected result.
  fd = open(filename, O_RDONLY);
  ensure(fd > 2);
  checked_read(fd, expected_message);
  
  // Ensure that we are at EOF.
  ensure(read(fd, &c, 1) == 0);

  // Close the file descriptor for the last time.
  ensure(close(fd) == 0);

  // Ideally we would remove this file now.
  // To avoid triggering unhandled syscall code, though, this is
  // omitted since most kernels will not implement remove().

  // Success!
  printf("SUCCESS\n");
  return 0;

  // Unused.
  (void)argc;
  (void)argv;
}
