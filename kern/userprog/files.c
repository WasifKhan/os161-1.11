#include <types.h>
#include <lib.h>
#include <thread.h>
#include <files.h>

int sys_write(int fd, const void* buf, size_t nbytes) {
	if (fd == 1) {
      kprintf((char*)buf);
   } else {
      kprintf("Something else %d\n", fd);
	}
   return 1;
}

int sys_open(const char* filename, int flags) {
	return 1;
}

int sys_close(int fd) {
	return 1;
}

int sys_read(int fd, void* buf, size_t buflen) {
	return 1;
}
