#ifndef _COREMAP_H_
#define _COREMAP_H_

#include <types.h>
#include <lib.h>
enum state {
   FIXED = -1,
   FREE = -2,
   USED = -3
};

struct coremap_entry {
   enum state curr_state;
   int contiguous_pages;
   paddr_t paddr;
   int entryNum;
};


int find_kpages (int k);

#endif
