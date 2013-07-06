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

extern numthreads;      // NOT SURE

void first_child_function (struct trapframe * tf, unsigned long l) {
   struct trapframe t;
   t = (*tf);

   // adjust the trap frame
   t.tf_epc = t.tf_epc + 4;
   t.tf_a3 = 0;
   t.tf_v0 = 0;
   
   kfree(tf);
   
   mips_usermode(&t);   // does not return
}


pid_t sys_fork(struct trapframe *tf, int * errno)
{
   int spl;
   spl = splhigh();
   //kprintf("starting\n");
   //initialize a trapframe, and do a deep copy of the parent trapframe 
   struct trapframe * tf_copy = kmalloc(sizeof(struct trapframe));
   
   if (tf_copy == NULL) {
      *errno = ENOMEM;
      splx(spl);
      return -1;
   }


   *tf_copy = (*tf);
   
   /*
   parameters to be passed into first_child_function, which 
   is passed into thread_fork
   */
   struct thread * t;
   unsigned long l = 1;

   /*
   don't want the child to run right away, so disable interrupts
   */

   //spl = splhigh();
   
   if (numthreads >= 300) {
      *errno = EAGAIN;
      splx(spl);
      return -1;
   }
   int retval;
   //kprintf("forking\n"); 
   retval = thread_fork("child", tf_copy, l, first_child_function, &t);  
   //kprintf("done forking\n");

   if (retval == ENOMEM) {
      *errno = ENOMEM;
      splx(spl);
      return -1;
   }
   //kprintf("theres enough memory\n");
   // copy the parents address space, set child's addrspace as the copy
   struct addrspace * childaddr;
   retval = as_copy(curthread->t_vmspace, &childaddr);
   //kprintf("as copied\n");
   if (retval == ENOMEM) {
      *errno = ENOMEM;
      splx(spl);
      return -1;
   }
   t->t_vmspace = childaddr;
   //kprintf("set child's Addrspace\n");
   as_activate(t->t_vmspace);
   //kprintf("activated addrspace\n");

   int index;
   for (index = 0; index < 100; index++)
   {
      if (curthread->fdTable[index] != NULL)
      {
         kprintf("starting file stuff\n");
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
         file->vn = curthread->fdTable[index]->vn;
         t->fdTable[index] = file;
         kprintf("done file stuff\n");
      }
      else
      {
         t->fdTable[index] = NULL;
      }
   }
   //kprintf("\n");
   splx(spl);
   return t->pid;
}

