#ifndef __PAGE_H__
#define __PAGE_H__

#include <stdint.h>

// Constants
#define PAGE_SIZE_BYTES (2 * 1024 * 1024)  
#define NUM_PHYSICAL_PAGES 128              

// Structure to represent a page
struct ppage {
    struct ppage *next;
    struct ppage *prev;
    void *physical_addr;
};

// Global pointer to the free page list
extern struct ppage *free_page_list;

// Function prototypes
void init_pfa_list(void);
struct ppage *allocate_physical_pages(unsigned int npages);
void free_physical_pages(struct ppage *ppage_list);
void print_pfa_state(void);  // For testing

#endif // __PAGE_H__