#include <types.h>
#include <lib.h>
#include <thread.h>
#include <fork.h>
#include <machine/trapframe.h>
#include <kern/errno.h>
#include <curthread.h>
#include <addrspace.h>
#include <machine/spl.h>
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
   //initialize a trapframe, and do a deep copy of the parent trapframe 
   struct trapframe * tf_copy = kmalloc(sizeof(struct trapframe));
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

   spl = splhigh();
   
   if (numthreads == 200) {
      *errno = EAGAIN;
      splx(spl);
      return -1;
   }

   int retval = thread_fork("child", tf_copy, l, first_child_function, &t);  
   if (retval = ENOMEM) {
      *errno = ENOMEM;
      splx(spl);
      return -1;
   }

   // copy the parents address space, set child's addrspace as the copy
   struct addrspace * childaddr;
   as_copy(curthread->t_vmspace, &childaddr);
   t->t_vmspace = childaddr;
   as_activate(t->t_vmspace);

   splx(spl);
   
   return t->pid;
}

