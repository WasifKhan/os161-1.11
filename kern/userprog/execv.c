#include <types.h>
#include <kern/unistd.h>
#include <lib.h>
#include <addrspace.h>
#include <thread.h>
#include <execv.h>
#include <vnode.h>
#include <kern/errno.h>
#include <curthread.h>
#include <vm.h>
#include <vfs.h>


int sys_execv(const char* program, char** args, int* errno)
{
	int ret;
	// simple error check
	// *************
	if (program == NULL)
	{
		*errno = EFAULT;
		return -1;
	}
	// ************

	// copies progname into kernel buffer
	// ***************
	char* fname = (char*)kmalloc(PATH_MAX);
	ret = copyinstr((userptr_t)program, fname, PATH_MAX, NULL);
	if (ret != 0)
	{
		*errno = EFAULT;
		return -1;
	}
	// opens executable	
	// ***************
	struct vnode* v;
	ret = vfs_open(fname, O_RDONLY, &v);
	if (ret != 0)
	{
		*errno = ret;
		return -1;
	}
	// **************


	// gets argc
	// **************
	int i = 0;
	while (args[i] != NULL)
	{
		i++;
	}
	int argc = i;
	// *************

	// copies all arguments into kernel buffer
	// *****************
	char** argv;
	argv= (char**)kmalloc(PATH_MAX);
	
	ret = copyin(args, argv, sizeof(userptr_t));
	if (ret != 0)
	{
		*errno = EFAULT;
		return -1;
	}
	
	for(i = 0; i < argc; i++)
	{
		argv[i]=(char*)kmalloc(PATH_MAX);
		ret = copyinstr((userptr_t)args[i],argv[i], PATH_MAX, NULL);
		if (ret != 0)
		{
			*errno = EFAULT;
			return -1;
		}
	}
	argv[argc] = NULL;
	// *******************

	// error check
	// ************
	if (curthread->t_vmspace != NULL)
	{
		as_destroy(curthread->t_vmspace);
		curthread->t_vmspace = NULL;
	}
	// **************



	// creates new address space and loads elf into it
	// **************	
	curthread->t_vmspace = as_create();
	if (curthread->t_vmspace == NULL)
	{
		vfs_close(v);
		*errno = ENOMEM;
		return -1;
	}

	vaddr_t entrypoint,stackptr;

	as_activate(curthread->t_vmspace);
	ret = load_elf(v,&entrypoint);
	if (ret != 0){
		vfs_close(v);
		*errno = ret;
		return -1;
	}
	vfs_close(v);
	ret = as_define_stack(curthread->t_vmspace,&stackptr);
	if (ret != 0)
	{
		*errno = ret;
		return -1;
	}
	// ********************

	// copies arguments into userstack and increments stack ptr
	// **************
	unsigned int pstack[argc];
	for (i = argc-1; i >= 0; i--)
	{
		
		int len = strlen(argv[i]) + 1;
		stackptr -= len;
		stackptr -= stackptr%4;
		ret = copyoutstr(argv[i], (userptr_t)stackptr, PATH_MAX, NULL);
		if (ret != 0)
		{
			*errno = EFAULT;
			return -1;
		}
		pstack[i] = stackptr;
	}
	pstack[argc] = (int)NULL;
	for (i = argc-1; i >= 0; i--)
	{
		stackptr -= 4;
		ret = copyout(&pstack[i], (userptr_t)stackptr, sizeof(pstack[i]));
		if (ret != 0)
		{
			*errno = EFAULT;
			return -1;
		}
	}
	
	// free unused memory	
	for (i = 0; i < argc; i++)
	{
		kfree(argv[i]);
	}
	kfree(argv);
	kfree(fname);
	// *******************


	// enters usermode, should not return
	// *******************
	md_usermode(argc, (userptr_t)stackptr, stackptr, entrypoint);
	// ******************
	
	// if usermode returns, that is an error
	// **********
	*errno = EINVAL;
	return -1;
	// ************
}
