enum state {
   FIXED,
   FREE,
   USED
};

struct coremap_entry {
   enum state curr_state;
   int contiguous_pages;
   paddr_t paddr;
   int entryNum;
};


int find_kpages (int k);
