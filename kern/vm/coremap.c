#include <types.h>
#include <coremap.h>
#include <addrspace.h>
#include <lib.h>

extern struct coremap_entry * coremap;
extern int freepages;
extern int num_pages;
extern int nextOut;

#define PAGE_SIZE 4096

void takePages(int index, int pages)
{
   int i;
   for (i = 0; i <= pages; i++)
   {
      coremap[index+i].curr_state = USED;
      coremap[index+i].entryNum = nextOut; // might have to change this to individual pages leaving
      coremap[index+i].contiguous_pages = -1;
      bzero(&coremap[index+i], PAGE_SIZE); 
   }
}

int find_kpages (int k) {
   int i;
   // might want to start at (num_pages - freepages - 1)
   for (i = (num_pages - freepages); i < num_pages; i++) 
   {
      if (coremap->curr_state == FREE &&
         coremap->contiguous_pages >= k)
         {
            takePages(i, k);
            nextOut ++;
            return i;
         }
      }
      return -1;
}



