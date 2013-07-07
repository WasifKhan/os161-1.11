/*
 * Sample/test code for running a user program.  You can use this for
 * reference when implementing the execv() system call. Remember though
 * that execv() needs to do more than this function does.
 */

#include <types.h>
#include <kern/unistd.h>
#include <kern/errno.h>
#include <lib.h>
#include <addrspace.h>
#include <thread.h>
#include <curthread.h>
#include <vm.h>
#include <vfs.h>
#include <test.h>

/*
 * Load program "progname" and start running it in usermode.
 * Does not return except on error.
 *
 * Calls vfs_open on progname and thus may destroy it.
 */
int
runprogram(char *progname, int argc, char** argv)
{

	struct vnode *v;
	vaddr_t entrypoint, stackptr;
	userptr_t varaddr;
	int result;

	/* Open the file. */
	result = vfs_open(progname, O_RDONLY, &v);
	if (result) {
		return result;
	}

	/* We should be a new thread. */
	assert(curthread->t_vmspace == NULL);

	/* Create a new address space. */
	curthread->t_vmspace = as_create();
	if (curthread->t_vmspace==NULL) {
		vfs_close(v);
		return ENOMEM;
	}

	/* Activate it. */
	as_activate(curthread->t_vmspace);

	/* Load the executable. */
	result = load_elf(v, &entrypoint);
	if (result) {
		/* thread_exit destroys curthread->t_vmspace */
		vfs_close(v);
		return result;
	}

	/* Done with the file now. */
	vfs_close(v);

	/* Define the user stack in the address space */
	result = as_define_stack(curthread->t_vmspace, &stackptr);
	if (result) {
		/* thread_exit destroys curthread->t_vmspace */
		return result;
	}

	char* user_argv[argc+1];

	int arg_index;
	size_t len;
	size_t copied;
	for (arg_index = 0; arg_index < argc; arg_index++) 
	{
		len = strlen(argv[arg_index]) + 1;

		// Adjust the string base
		stackptr -= len;
		stackptr -= stackptr%4;

		result = copyoutstr(argv[arg_index], (userptr_t) stackptr, len, &copied);
		if (result != 0)
		{
			return -1;
		}

		user_argv[arg_index] = (char*) stackptr;
	}

	user_argv[arg_index] = NULL;
	stackptr -= 4*(argc+1);
	result = copyout(user_argv, (userptr_t) stackptr, 4*(argc+1));
	assert(result == 0);

	varaddr = (userptr_t) stackptr;

	stackptr -= 4;
	stackptr -= stackptr%8;

	/* Warp to user mode. */
	md_usermode(argc /*argc*/, varaddr /*userspace addr of argv*/,
		    stackptr, entrypoint);
	
	/* md_usermode does not return */
	panic("md_usermode returned\n");
	return EINVAL;
}

