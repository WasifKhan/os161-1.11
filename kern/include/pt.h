#ifndef _PT_H_
#define _PT_H_

#include <types.h>
#include <lib.h>

struct pt_entry {
	vaddr_t vaddr;
	paddr_t paddr;
	int nextBoot;
};

paddr_t inMem(vaddr_t vaddr);

#endif
