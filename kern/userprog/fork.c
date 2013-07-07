#include <types.h>
#include <lib.h>
#include <thread.h>
#include <fork.h>
#include <machine/trapframe.h>
#include <kern/errno.h>
#include <curthread.h>
#include <addrspace.h>
#include <machine/spl.h>

// include files.h to use fdesc structure when making child
#include <files.h>
// ******
// include vnote.h for vfs_open
#include <vnode.h>
// ******

extern numthreads;

void first_child_function (struct trapframe * tf, struct addrspace * addr) { // unsigned long l) {
   int spl;
   spl = splhigh();
   struct trapframe t;
   t = (*tf);

/*   
   int index;
   for (index = 3; index < 100; index++)
   {
      if (curthread->fdTable[index] != NULL)
      {
         kprintf("Test1\n");
         kprintf("Filename is: %s\n", curthread->fdTable[index]->filename);
         struct vnode* File = NULL;
         int ret = vfs_open(curthread->fdTable[index]->filename, curthread->fdTable[index]->flags, &File);
         kprintf("This should return2\n");
         assert(ret == 0);
         curthread->fdTable[index]->vn = File;
         kprintf("This should return3\n");
      }
   }
*/
   // adjust the trap frame
   t.tf_epc = t.tf_epc + 4;
   t.tf_a3 = 0;
   t.tf_v0 = 0;
  
   curthread->t_vmspace = addr;
   assert (curthread->t_vmspace != NULL);
   as_activate(curthread->t_vmspace);
   //as_activate(curthread->t_vmspace);
   
   kfree(tf);
   splx(spl);
   mips_usermode(&t);   // does not return
}


pid_t sys_fork(struct trapframe *tf, int * errno)
{
   int spl;
   spl = splhigh();
   //initialize a trapframe, and do a deep copy of the parent trapframe 
   struct trapframe * tf_copy = kmalloc(sizeof(struct trapframe));
   
   if (tf_copy == NULL) {
      *errno = ENOMEM;
      splx(spl);
      return -1;
   }
   
   *tf_copy = (*tf);
   
   struct thread * t;
   //unsigned long l = 1;
   int retval;

   if (numthreads > 300) {
      *errno = EAGAIN;
      splx(spl);
      return -1;
   }
   
   struct addrspace * childaddr;
   as_copy(curthread->t_vmspace, &childaddr);
   if (childaddr == NULL) {
      *errno = ENOMEM;
      splx(spl);
      return -1;
   }
   retval = thread_fork("child", tf_copy, childaddr, first_child_function, &t);  
   //kprintf("done forking\n");

   if (retval == ENOMEM) {
      *errno = ENOMEM;
      splx(spl);
      return -1;
   }
   //kprintf("theres enough memory\n");
   // copy the parents address space, set child's addrspace as the copy
  /* 
   int index;
   for (index = 0; index < 100; index++) {
      t->fdTable[index] = NULL;
   }
   */
   //t->t_vmspace = childaddr;
   //kprintf("set child's Addrspace\n");
   //as_activate(t->t_vmspace);
   //kprintf("activated addrspace\n");
   /*int index;
   for (index = 0; index < 100; index++)
   {
      if (curthread->fdTable[index] != NULL)
      {
         kprintf("\nindex is: %d\n", index);
         struct fdesc* file = (struct fdesc*) kmalloc(sizeof (struct fdesc));
         if (file == NULL) {
            *errno = ENOMEM;
            splx(spl);
            return -1;
         }
         file->flags = curthread->fdTable[index]->flags;
         //curthread->fdTable[index] = 
         file->offset = curthread->fdTable[index]->offset;
         file->ref_count = curthread->fdTable[index]->ref_count;
         struct vnode* File;
         char* fileInput = NULL;
         fileInput = kstrdup(curthread->fdTable[index]->filename);
         file->filename = fileInput;
         file->vn = File;
         t->fdTable[index] = file;
      }
      else
      {
         t->fdTable[index] = NULL;
      }
   }
   */
   splx(spl);
   return t->pid;
}

