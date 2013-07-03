#ifndef _FILES_H_
#define _FILES_H_


#include <types.h>
#include <lib.h>
#include <kern/unistd.h>

struct fdesc
{
	char* name;
	int flags;
	off_t offset;
	int ref_count;
	struct lock* lock;
	struct vnode* vn;
};
int sys_write(int fd, const void* buf, size_t nbytes);
int sys_open(const char* filename, int flags);
int sys_close(int fd);
int sys_read(int fd, void* buf, size_t buflen);

#endif
