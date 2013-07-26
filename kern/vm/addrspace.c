#include <types.h>
#include <kern/errno.h>
#include <lib.h>
#include <thread.h>
#include <curthread.h>
#include <addrspace.h>
#include <vm.h>
#include <machine/spl.h>
#include <machine/tlb.h>
#include <array.h>
#include "coremap.h"


#define PAGE_SIZE 4096

#include <vm_tlb.h>

#define DUMBVM_STACKPAGES    12

struct coremap_entry * coremap = NULL;
int freepages;
int num_pages; 
pid_t curPid = -1;

int totalFaults = 0;
int faultsWithFree = 0;
int faultsWithReplace = 0;
int invalidations = 0;
struct lock * coreEntryLock;


int nextOut;
// ****************

void
vm_bootstrap(void)
{
	int spl = splhigh();
	u_int32_t free;
	u_int32_t first;
    u_int32_t last;
    ram_getsize(&first, &last);
    //num_pages = last / PAGE_SIZE; 
    // ensure that first and last do not land in the middle of a page 
   
    while (last % PAGE_SIZE != 0) {
       last--;      
    }
	num_pages = last / PAGE_SIZE - 1;
   
    while (first % PAGE_SIZE != 0) {
       first ++;
    }

    // initialize the coremap
    coremap = (struct coremap_entry*) PADDR_TO_KVADDR (first);
    free = first + num_pages * sizeof(struct coremap_entry); 

   // have to make sure that free begins at the first full free page 
  
   while (free % PAGE_SIZE != 0) {
      free++;
   }

   // fill the coremap with coremap_entrys
   
   // first, the fixed pages {kernel stuff + the coremap itself}
   paddr_t curraddr = 0;
   int page;
   
   int fixedpages = free / PAGE_SIZE;
   for (page = 0; page <= fixedpages; page++) {
      coremap[page].curr_state = FIXED;
      coremap[page].contiguous_pages = -1;
      coremap[page].paddr = curraddr;
      coremap[page].entryNum = -1;
      curraddr+= PAGE_SIZE;
   }

   kprintf("\n\n First index %d and first address %x \n\n", page, curraddr);
   assert (curraddr % PAGE_SIZE == 0); 
   //freepages = (last - free) / PAGE_SIZE;
   freepages = num_pages - page;
   
   for (; page < num_pages; page ++) {
	  assert(coremap[page].curr_state != FIXED);
	  coremap[page].curr_state = FREE;
      coremap[page].contiguous_pages = freepages;
      coremap[page].paddr = curraddr;
      coremap[page].entryNum = -1;
      curraddr+= PAGE_SIZE;
      freepages--;
   }

   nextOut = 0;
   coreEntryLock = lock_create("coremapEntryLock");
   splx(spl);
}
// ****************


// ****************
//get physical pages
static
paddr_t
getppages(unsigned long npages)
{
	int spl;
	paddr_t addr;

	spl = splhigh();

	addr = ram_stealmem(npages);
	
	splx(spl);
	return addr;
}
// ****************

// ****************
/* Allocate/free some kernel-space virtual pages */
vaddr_t 
alloc_kpages(int npages)
{
	
   if (coremap == NULL) {	
      paddr_t pa;
   	  pa = getppages(npages);
   	  if (pa==0) {
		  return 0;
   	  }
	  return PADDR_TO_KVADDR(pa);
   
   } else {
	  lock_acquire(coreEntryLock);
      int result = find_kpages(npages);
      
      if (result == -1) {
         lock_release(coreEntryLock);
		 kprintf("perhaps not nuff memory\n");
		 return 0;  
      
	  } else {
         kprintf("Phys addr is %x \n", coremap[result].paddr);
		 kprintf("Virt addr is %x \n", PADDR_TO_KVADDR(coremap[result].paddr));
		 lock_release(coreEntryLock);
		 return PADDR_TO_KVADDR(coremap[result].paddr);   
      }
   }
}
// ****************


// ****************
   
/* sets the coremap entry to FREE and zero-fills the page */
void set_free (int i) {
   assert (coremap[i].curr_state != FIXED);
   coremap[i].curr_state = FREE;
   coremap[i].entryNum = -1;
   bzero(PADDR_TO_KVADDR(coremap[i].paddr), PAGE_SIZE);
}


/* this function sets the contiguous_pages field for the coremap_entrys being freed */

void set_contiguousPages (int firstIndex, int lastIndex, int contPages) {
   int i;
   int currentContPages = contPages;

   for (i = firstIndex; i <= lastIndex; i++) {
      coremap[i].contiguous_pages = currentContPages;
      currentContPages--;
   }

}

void 
free_kpages(vaddr_t addr)
{
  /*	if (coremap != NULL) {
		lock_acquire(coreEntryLock);
		paddr_t phys_addr = addr - 0x80000000;
		int page = phys_addr / PAGE_SIZE;
   		//assert(coremap[page].curr_state != FIXED);
		int startingPage;
   		// find the first page to free
	
	
	   	for (startingPage = 0; startingPage < num_pages; startingPage++) {
    	  if (coremap[startingPage].paddr == phys_addr) {
        	 break;   
     	  }
   		}
   		//int i = startingPage;
   		int i = page;
	   	int contigPages = 1;
   		int eNum = coremap[i].entryNum;
   
	    //find number of contiguous pages, free along the way
  		while (coremap[i].entryNum == eNum) {
	      set_free(i);
    	  contigPages++;
   	   	  i++;
  		 }

	   // check if the number of contiguous pages goes beyond what we are freeing
	   if (coremap[i].curr_state == FREE) {
   		   contigPages += coremap[i].contiguous_pages;   
  	   }
   	
    	set_contiguousPages (startingPage, i-1, contigPages);
		lock_release(coreEntryLock);
	}*/
}
// ****************

// ****************
int
vm_fault(int faulttype, vaddr_t faultaddress)
{
	vaddr_t vbase1, vtop1, vbase2, vtop2, stackbase, stacktop;
	paddr_t paddr;
	int i;
	u_int32_t ehi, elo;
	struct addrspace *as;
	int spl;

	spl = splhigh();

	faultaddress &= PAGE_FRAME;

	DEBUG(DB_VM, "dumbvm: fault: 0x%x\n", faultaddress);


	switch (faulttype) {
	    case VM_FAULT_READONLY:
	    	thread_exit();
		case VM_FAULT_READ:
	    case VM_FAULT_WRITE:
			break;
	    default:
			splx(spl);
			return EINVAL;
	}

	as = curthread->t_vmspace;
	if (as == NULL) {
		/*
		 * No address space set up. This is probably a kernel
		 * fault early in boot. Return EFAULT so as to panic
		 * instead of getting into an infinite faulting loop.
		 */
		return EFAULT;
	}

	/* Assert that the address space has been set up properly. */
	assert(as->as_vbase1 != 0);
	assert(as->as_pbase1 != 0);
	assert(as->as_npages1 != 0);
	assert(as->as_vbase2 != 0);
	assert(as->as_pbase2 != 0);
	assert(as->as_npages2 != 0);
	assert(as->as_stackpbase != 0);
	assert((as->as_vbase1 & PAGE_FRAME) == as->as_vbase1);
	assert((as->as_pbase1 & PAGE_FRAME) == as->as_pbase1);
	assert((as->as_vbase2 & PAGE_FRAME) == as->as_vbase2);
	assert((as->as_pbase2 & PAGE_FRAME) == as->as_pbase2);
	assert((as->as_stackpbase & PAGE_FRAME) == as->as_stackpbase);


	vbase1 = as->as_vbase1;
	vtop1 = vbase1 + as->as_npages1 * PAGE_SIZE;
	vbase2 = as->as_vbase2;
	vtop2 = vbase2 + as->as_npages2 * PAGE_SIZE;
	stackbase = USERSTACK - DUMBVM_STACKPAGES * PAGE_SIZE;
	stacktop = USERSTACK;

	if (faultaddress >= vbase1 && faultaddress < vtop1) {
		paddr = (faultaddress - vbase1) + as->as_pbase1;
	}
	else if (faultaddress >= vbase2 && faultaddress < vtop2) {
		paddr = (faultaddress - vbase2) + as->as_pbase2;
	}
	else if (faultaddress >= stackbase && faultaddress < stacktop) {
		paddr = (faultaddress - stackbase) + as->as_stackpbase;
	}
	else {
		splx(spl);
		return EFAULT;
	}

	/* make sure it's page-aligned */
	assert((paddr & PAGE_FRAME)==paddr);

	for (i=0; i<NUM_TLB; i++) {
		TLB_Read(&ehi, &elo, i);
		if (elo & TLBLO_VALID) {
			continue;
		}
		ehi = faultaddress;
		elo = paddr | TLBLO_DIRTY | TLBLO_VALID;
		DEBUG(DB_VM, "dumbvm: 0x%x -> 0x%x\n", faultaddress, paddr);
		TLB_Write(ehi, elo, i);
		splx(spl);
		faultsWithFree++;
		totalFaults++;
		return 0;
	}

// *********
// 	replaces next victim when TLB is full
	int nextVictim = tlb_get_rr_victim();
	ehi = faultaddress;
	elo = paddr | TLBLO_DIRTY | TLBLO_VALID;
	TLB_Write(ehi, elo, nextVictim);
	splx(spl);
	faultsWithReplace++;
	totalFaults++;
// *************
	return 0;
	
//	kprintf("dumbvm: Ran out of TLB entries - cannot handle page fault\n");
}
// ****************

// ****************
// ****************

// ****************
// ****************

// ****************
struct addrspace *
as_create(void)
{
	struct addrspace *as = kmalloc(sizeof(struct addrspace));
	if (as==NULL) {
		return NULL;
	}

	as->as_vbase1 = 0;
	// *************
	// Making the code section read-only
	as->as_pbase1 = 0;
	// ************
	as->as_npages1 = 0;
	as->as_vbase2 = 0;
	as->as_pbase2 = 0;
	as->as_npages2 = 0;
	as->as_stackpbase = 0;

	return as;
}
// ****************

// ****************
int
as_copy(struct addrspace *old, struct addrspace **ret)
{
	struct addrspace *new;

	new = as_create();
	if (new==NULL) {
		return ENOMEM;
	}

	new->as_vbase1 = old->as_vbase1;
	new->as_npages1 = old->as_npages1;
	new->as_vbase2 = old->as_vbase2;
	new->as_npages2 = old->as_npages2;

	if (as_prepare_load(new)) {
		as_destroy(new);
		return ENOMEM;
	}

	assert(new->as_pbase1 != 0);
	assert(new->as_pbase2 != 0);
	assert(new->as_stackpbase != 0);

	memmove((void *)PADDR_TO_KVADDR(new->as_pbase1),
		(const void *)PADDR_TO_KVADDR(old->as_pbase1),
		old->as_npages1*PAGE_SIZE);

	memmove((void *)PADDR_TO_KVADDR(new->as_pbase2),
		(const void *)PADDR_TO_KVADDR(old->as_pbase2),
		old->as_npages2*PAGE_SIZE);

	memmove((void *)PADDR_TO_KVADDR(new->as_stackpbase),
		(const void *)PADDR_TO_KVADDR(old->as_stackpbase),
		DUMBVM_STACKPAGES*PAGE_SIZE);
	
	*ret = new;
	return 0;
}
// ****************

// ****************
void
as_destroy(struct addrspace *as)
{
	kfree(as);
}
// ****************

// ****************
void
as_activate(struct addrspace *as)
{
	// makes sure a context switch doesn't occur if switching to the same process
	if (curPid != curthread->pid)
	{
		int i, spl;

		(void)as;

		spl = splhigh();

		for (i=0; i<NUM_TLB; i++) {
			TLB_Write(TLBHI_INVALID(i), TLBLO_INVALID(), i);
		}
		invalidations++;
		splx(spl);

		curPid = curthread->pid;
	}
}
// ****************

/*
 * Set up a segment at virtual address VADDR of size MEMSIZE. The
 * segment in memory extends from VADDR up to (but not including)
 * VADDR+MEMSIZE.
 *
 * The READABLE, WRITEABLE, and EXECUTABLE flags are set if read,
 * write, or execute permission should be set on the segment. At the
 * moment, these are ignored. When you write the VM system, you may
 * want to implement them.
 */


// ****************
int
as_define_region(struct addrspace *as, vaddr_t vaddr, size_t sz,
		 int readable, int writeable, int executable)
{
	size_t npages; 

	/* Align the region. First, the base... */
	sz += vaddr & ~(vaddr_t)PAGE_FRAME;
	vaddr &= PAGE_FRAME;

	/* ...and now the length. */
	sz = (sz + PAGE_SIZE - 1) & PAGE_FRAME;

	npages = sz / PAGE_SIZE;

	/* We don't use these - all pages are read-write */
	(void)readable;
	(void)writeable;
	(void)executable;

	if (as->as_vbase1 == 0) {
		as->as_vbase1 = vaddr;
		as->as_npages1 = npages;
		
		// ************
		// making code segment read only
		if (as->as_pbase1 == (as->as_pbase1 | TLBLO_DIRTY))
		{
			as->as_pbase1 -= TLBLO_DIRTY;
		}
		// ************
		return 0;
	}

	if (as->as_vbase2 == 0) {
		as->as_vbase2 = vaddr;
		as->as_npages2 = npages;
		return 0;
	}

	/*
	 * Support for more than two regions is not available.
	 */
	kprintf("dumbvm: Warning: too many regions\n");
	return EUNIMP;
}
// ****************

// ****************
int
as_prepare_load(struct addrspace *as)
{
	assert(as->as_pbase1 == 0);
	assert(as->as_pbase2 == 0);
	assert(as->as_stackpbase == 0);

	as->as_pbase1 = getppages(as->as_npages1);
	if (as->as_pbase1 == 0) {
		return ENOMEM;
	}

	as->as_pbase2 = getppages(as->as_npages2);
	if (as->as_pbase2 == 0) {
		return ENOMEM;
	}

	as->as_stackpbase = getppages(DUMBVM_STACKPAGES);
	if (as->as_stackpbase == 0) {
		return ENOMEM;
	}

	return 0;
}
// ****************

// ****************
int
as_complete_load(struct addrspace *as)
{
	(void)as;
	return 0;
}
// ****************

// ****************
int
as_define_stack(struct addrspace *as, vaddr_t *stackptr)
{
	assert(as->as_stackpbase != 0);

	*stackptr = USERSTACK;
	
	return 0;
}
// ****************

