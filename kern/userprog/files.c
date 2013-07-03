#include <types.h>
#include <lib.h>
#include <thread.h>
#include <files.h>
#include <vfs.h>
#include <curthread.h>
#include <lib.h>
#include <kern/limits.h>


int findFree()
{
	int counter = 3;
	while (curthread->fdTable[counter]!=NULL)
	{
		counter ++;
	}
	return counter;
}

int sys_write(int fd, const void* buf, size_t nbytes) {
	int counter;
	
	struct fdesc stdInput;
	char* consoleIn = NULL;
	consoleIn = kstrdup("con:");
	int mode = O_RDONLY;
	int off = 0;
	int ref = 0;
	struct vnode* stdIn;
	int ret = vfs_open(consoleIn, mode, &stdIn);
	assert (ret==0);
	stdInput.name = consoleIn;
	stdInput.flags = mode;
	stdInput.offset = off;
	stdInput.ref_count = ref;
	stdInput.vn = stdIn;
	curthread->fdTable[0]= &stdInput;

	struct fdesc stdOutput;
	char* consoleOut = NULL;
	consoleOut = kstrdup("con:");
	mode = O_WRONLY;
	struct vnode* stdOut;
	vfs_open(consoleOut, mode, &stdOut);
	stdOutput.name = consoleOut;
	stdOutput.flags = mode;
	stdOutput.offset = off;
	stdOutput.ref_count = ref;
	stdOutput.vn = stdOut;
	curthread->fdTable[1] = &stdOutput;

	struct fdesc stdError;
	char* consoleErr = NULL;
	consoleErr = kstrdup("con:");
	mode = O_WRONLY;
	struct vnode* stdErr;
	vfs_open(consoleErr, mode, &stdErr);
	stdError.name = consoleErr;
	stdError.flags = mode;
	stdError.offset = off;
	stdError.ref_count = ref;
	stdError.vn = stdErr;
	curthread->fdTable[2] = &stdError;

	for (counter = 3; counter < 100; counter++)
	{
		(curthread->fdTable)[counter] = NULL;
	}
	
	if (fd == 1) {
      kprintf((char*)buf);
   } else {
      kprintf("Something else %d\n", fd);
	}
   return 1;
}

int sys_open(const char* filename, int flags) {

	// finds open fd
	int freeFd = findFree();
	
	size_t len;
	// copies string name for vfs_open
	char* kfilename = kmalloc(PATH_MAX);

	copyinstr((const_userptr_t)filename, kfilename, PATH_MAX, &len);

	// creates file vnode
	struct vnode* File;

	// vfs_open on File
	int ret = vfs_open(kfilename, flags, &File);
	assert (ret == 0);
	//create the file descriptor
	struct fdesc openFile;
	openFile.name = kfilename;
	openFile.flags = flags;
	openFile.offset = 0;
	openFile.ref_count = 0;
	openFile.vn = File;
	curthread->fdTable[freeFd]= &openFile;

	return freeFd;
}

int sys_close(int fd) {
	// need to free the name field in fdesc struct
	return 1;
}

int sys_read(int fd, void* buf, size_t buflen) {
	return 1;
}
