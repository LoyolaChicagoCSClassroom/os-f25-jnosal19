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

// Page directory entry structure
struct page_directory_entry {
    uint32_t present       : 1;   // Page present in memory
    uint32_t rw            : 1;   // Read-only if clear, R/W if set
    uint32_t user          : 1;   // Supervisor only if clear
    uint32_t writethru     : 1;   // Write-through caching
    uint32_t cachedisabled : 1;   // Cache disabled
    uint32_t accessed      : 1;   // Has been accessed
    uint32_t pagesize      : 1;   // 0 = 4KB pages, 1 = 4MB pages
    uint32_t ignored       : 2;   // Ignored bits
    uint32_t os_specific   : 3;   // Available for OS use
    uint32_t frame         : 20;  // Physical address >> 12
};

// Page table entry structure
struct page {
    uint32_t present    : 1;   // Page present in memory
    uint32_t rw         : 1;   // Read-only if clear, readwrite if set
    uint32_t user       : 1;   // Supervisor level only if clear
    uint32_t accessed   : 1;   // Has been accessed
    uint32_t dirty      : 1;   // Has been written to
    uint32_t unused     : 7;   // Unused bits
    uint32_t frame      : 20;  // Physical address >> 12
};

void *map_pages(void *vaddr, struct ppage *pglist, struct page_directory_entry *pd);
void enable_paging(void);

#endif // __PAGE_H__