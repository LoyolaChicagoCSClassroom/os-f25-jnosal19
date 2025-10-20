#include "page.h"
#include "rprintf.h"

// Static array of physical page structures
static struct ppage physical_page_array[NUM_PHYSICAL_PAGES];

// Global pointer to the head of the free page list
struct ppage *free_page_list = 0;

void init_pfa_list(void) {
    for (int i = 0; i < NUM_PHYSICAL_PAGES; i++) {
        // Set the physical address for this page (each page is 2MB)
        physical_page_array[i].physical_addr = (void *)((uintptr_t)i * PAGE_SIZE_BYTES);
        
        // Link to next page (NULL if this is the last page)
        physical_page_array[i].next = (i < NUM_PHYSICAL_PAGES - 1) ? &physical_page_array[i + 1] : 0;
        
        // Link to previous page (NULL if this is the first page)
        physical_page_array[i].prev = (i > 0) ? &physical_page_array[i - 1] : 0;
    }
    
    // Set the head of the free list to the first page
    free_page_list = &physical_page_array[0];
}

struct ppage *allocate_physical_pages(unsigned int npages) {
    // Check for invalid input or empty free list
    if (!free_page_list || npages == 0)
        return 0;
    
    // Start of the allocated list
    struct ppage *alloc_head = free_page_list;
    struct ppage *alloc_tail = alloc_head;
    
    // Walk through the free list to find npages
    for (unsigned int i = 1; i < npages && alloc_tail->next; i++) {
        alloc_tail = alloc_tail->next;
    }
    
    // Detach the allocated pages from the free list
    free_page_list = alloc_tail->next;
    if (free_page_list)
        free_page_list->prev = 0;
    
    // Terminate the allocated list
    alloc_tail->next = 0;
    
    return alloc_head;
}


void free_physical_pages(struct ppage *ppage_list) {
    if (!ppage_list)
        return;
    
    // Find the tail of the list being freed
    struct ppage *tail = ppage_list;
    while (tail->next)
        tail = tail->next;
    
    // Link the freed list to the current free list
    if (free_page_list)
        free_page_list->prev = tail;
    tail->next = free_page_list;
    
    // The freed list becomes the new head
    ppage_list->prev = 0;
    free_page_list = ppage_list;
}


extern int putc(int c);  

void print_pfa_state(void) {
    struct ppage *cur = free_page_list;
    esp_printf(putc, "\nFree list:\n");
    while (cur) {
        esp_printf(putc, "  Page @ %x -> %x\n", cur, cur->physical_addr);
        cur = cur->next;
    }
    esp_printf(putc, "(end of free list)\n");
}