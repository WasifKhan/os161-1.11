#include <pt.h>
#include <addrspace.h>
#include <curthread.h>
#include <thread.h>


#define PAGE_SIZE 4096

extern int num_pages;
// ****************
// This function checks if a virtual address is in the page table or not
// If it is - return the paddr associated - otherwise return 0
paddr_t inMem(vaddr_t vaddr)
{
	vaddr_t newV = vaddr;
	while (newV % PAGE_SIZE != 0)
	{
		newV --;
	}
	int i;
	for (i = 0; i < num_pages; i++)
	{
		if ((curthread->pageTable[i]).vaddr == vaddr)
		{
			return curthread->pageTable[i].paddr;
		}
	}
	return 0;
}
// ****************
