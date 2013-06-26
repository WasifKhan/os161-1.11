#include <types.h>
#include <lib.h>

int sys_write(int fd, const void* buf, size_t nbytes);
int sys_open(const char* filename, int flags);
int sys_close(int fd);
int sys_read(int fd, void* buf, size_t buflen);
