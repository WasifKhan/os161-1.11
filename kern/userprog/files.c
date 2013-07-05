#include <types.h>
#include <lib.h>
#include <thread.h>
#include <files.h>
#include <vfs.h>
#include <curthread.h>
#include <lib.h>
#include <kern/limits.h>
#include <uio.h>
#include <synch.h>
#include <vnode.h>
#include <kern/errno.h>
#include <addrspace.h>

	// Used to initialize standard in/out/err for a given thread (moved from thread_create because of bootstrap problem
	// ******************
	
void openConsoles()
{
	int counter;
	
	struct fdesc* stdInput = kmalloc(sizeof(struct fdesc));
	char* consoleIn = NULL;
	consoleIn = kstrdup("con:");
	int mode = O_RDONLY;
	int off = 0;
	int ref = 0;
	struct vnode* stdIn;
	int ret = vfs_open(consoleIn, mode, &stdIn);
	assert (ret==0);
	stdInput->name = consoleIn;
	stdInput->flags = mode;
	stdInput->offset = off;
	stdInput->ref_count = ref;
	stdInput->vn = stdIn;
	stdInput->lock = lock_create(consoleIn);
	curthread->fdTable[0]= stdInput;

	struct fdesc* stdOutput = kmalloc(sizeof(struct fdesc));
	char* consoleOut = NULL;
	consoleOut = kstrdup("con:");
	mode = O_WRONLY;
	struct vnode* stdOut;
	vfs_open(consoleOut, mode, &stdOut);
	stdOutput->name = consoleOut;
	stdOutput->flags = mode;
	stdOutput->offset = off;
	stdOutput->ref_count = ref;
	stdOutput->vn = stdOut;
	stdOutput->lock = lock_create(consoleOut);
	curthread->fdTable[1] = stdOutput;

	struct fdesc* stdError = kmalloc(sizeof(struct fdesc));
	char* consoleErr = NULL;
	consoleErr = kstrdup("con:");
	mode = O_WRONLY;
	struct vnode* stdErr;
	vfs_open(consoleErr, mode, &stdErr);
	stdError->name = consoleErr;
	stdError->flags = mode;
	stdError->offset = off;
	stdError->ref_count = ref;
	stdError->vn = stdErr;
	stdError->lock = lock_create(consoleErr);
	curthread->fdTable[2] = stdError;
}

int findFree()
{
	int counter = 3;
	while (curthread->fdTable[counter]!=NULL)
	{
		counter ++;
	}
	return counter;
}


int sys_open(const char* filename, int flags) {
// NEED A CHECK TO SEE IF FILE ALREADY EXISTS

	// finds open fd
	int freeFd = findFree();
	
	size_t len;
	// copies string name for vfs_open
	char* kfilename = kmalloc(PATH_MAX);

	copyinstr((const_userptr_t)filename, kfilename, PATH_MAX, &len);

	// creates file vnode
	struct vnode* File = NULL;

	// vfs_open on File
	int ret = vfs_open(kfilename, flags, &File);
	assert (ret == 0);
	//create the file descriptor
	struct fdesc* openFile = (struct fdesc*)kmalloc(sizeof(struct fdesc));
	openFile->name = kfilename;
	openFile->flags = flags;
	openFile->offset = 0;
	openFile->ref_count = 0;
	openFile->vn = File;
	openFile->isOpen = 1;
	openFile->lock = lock_create(kfilename);
	curthread->fdTable[freeFd]= openFile;
	return freeFd;
}

int sys_close(int fd) {
	vfs_close(curthread->fdTable[fd]->vn);
	lock_destroy(curthread->fdTable[fd]->lock);
	kfree((curthread->fdTable[fd])->name);
	kfree((curthread->fdTable[fd]));
	curthread->fdTable[fd] = NULL;
	return 0;
}

int sys_write(int fd, const void* buf, size_t nbytes) {
	if (curthread->init == 0)
	{
		openConsoles();
		curthread->init = 1;
	}
	// Error check for valid fd
	if (fd < 0 || fd > 99)
	{
		return EBADF;
	}

	int ret;
	struct fdesc* curFile = curthread->fdTable[fd];
	assert(curFile != NULL);
	// Error check for RDONLY
	if (curFile->flags == O_RDONLY)
	{
		return EBADF;
	}

	// set up uio for writing
	struct uio u;

	u.uio_iovec.iov_un.un_ubase = (userptr_t)buf;
	u.uio_iovec.iov_len = nbytes;

	u.uio_offset = curFile->offset;
	u.uio_resid = nbytes;
	u.uio_segflg = UIO_SYSSPACE;
	u.uio_rw = UIO_WRITE;
	u.uio_space = NULL;

	lock_acquire(curFile->lock);
	assert(curFile->vn != NULL);

	ret = VOP_WRITE(curFile->vn, &u);
	assert(ret >= 0);
	lock_release(curFile->lock);	

	ret = nbytes - u.uio_resid;

	curFile->offset = u.uio_offset;
   return ret;
}
int sys_read(int fd, void* buf, size_t buflen) {
	int ret;
	if (fd < 0 || fd > 99)
	{
		return EBADF;
	}
	struct fdesc* curFile = curthread->fdTable[fd];
	if (curFile->flags == O_WRONLY)
	{
		return EBADF;
	}
	struct uio u;
	// set up uio for reading
	u.uio_iovec.iov_un.un_ubase = buf;
	u.uio_iovec.iov_len = buflen;
	u.uio_offset = curFile->offset;

	u.uio_resid = buflen;
	u.uio_segflg = UIO_USERSPACE;
	u.uio_rw = UIO_READ;
	u.uio_space = curthread->t_vmspace;
	lock_acquire(curFile->lock);
	VOP_READ(curFile->vn, &u);
	lock_release(curFile->lock);

	ret = buflen - u.uio_resid;
	curFile->offset = buflen - u.uio_offset;
	return ret;
}
