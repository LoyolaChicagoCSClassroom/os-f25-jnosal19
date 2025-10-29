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

void *map_pages(void *vaddr, struct ppage *pglist, struct page_directory_entry *pd) {
    struct ppage *current_page = pglist;
    uint32_t virt_addr = (uint32_t)vaddr;
    
    // Iterate through each page in the list
    while (current_page != NULL) {
        // Extract page directory index (bits 31-22)
        uint32_t pd_index = virt_addr >> 22;
        
        // Extract page table index (bits 21-12)
        uint32_t pt_index = (virt_addr >> 12) & 0x3FF;
        
        // Check if page directory entry exists
        if (!pd[pd_index].present) {
            // Set up page directory entry to point to our page table
            pd[pd_index].present = 1;
            pd[pd_index].rw = 1;
            pd[pd_index].user = 0;  
            pd[pd_index].writethru = 0;
            pd[pd_index].cachedisabled = 0;
            pd[pd_index].accessed = 0;
            pd[pd_index].pagesize = 0;  // 4KB pages
            pd[pd_index].ignored = 0;
            pd[pd_index].os_specific = 0;
            // Point to page table (physical address >> 12)
            pd[pd_index].frame = ((uint32_t)pt) >> 12;
        }
        
        // Set up page table entry
        pt[pt_index].present = 1;
        pt[pt_index].rw = 1;
        pt[pt_index].user = 0; 
        pt[pt_index].accessed = 0;
        pt[pt_index].dirty = 0;
        pt[pt_index].unused = 0;
        // Set physical address (physical address >> 12)
        pt[pt_index].frame = ((uint32_t)current_page->physical_addr) >> 12;
        
        // Move to next page
        current_page = current_page->next;
        virt_addr += 4096;  // Move to next 4KB page
    }
    
    return vaddr;
}

void enable_paging(void) {
    extern struct page_directory_entry pd[];
    extern char _end_kernel;
    
    // Initialize page directory to all zeros
    for (int i = 0; i < 1024; i++) {
        pd[i].present = 0;
    }
    
    // Identity map kernel from 0x100000 to _end_kernel
    uint32_t kernel_start = 0x100000;
    uint32_t kernel_end = (uint32_t)&_end_kernel;
    
    // Round up to nearest page boundary
    kernel_end = (kernel_end + 4095) & ~4095;
    
    esp_printf(putc, "Mapping kernel from %x to %x\n", kernel_start, kernel_end);
    
    // Map each page of the kernel
    for (uint32_t addr = kernel_start; addr < kernel_end; addr += 4096) {
        struct ppage tmp;
        tmp.next = NULL;
        tmp.prev = NULL;
        tmp.physical_addr = (void *)addr;
        map_pages((void *)addr, &tmp, pd);
    }
    
    // Identity map the stack
    uint32_t esp;
    __asm__ __volatile__("mov %%esp, %0" : "=r"(esp));
    
    // Round down to page boundary
    uint32_t stack_page = esp & ~4095;
    esp_printf(putc, "Mapping stack at %x\n", stack_page);
    
    struct ppage stack_tmp;
    stack_tmp.next = NULL;
    stack_tmp.prev = NULL;
    stack_tmp.physical_addr = (void *)stack_page;
    map_pages((void *)stack_page, &stack_tmp, pd);
    
    // Identity map video memory at 0xB8000
    esp_printf(putc, "Mapping video memory at %x\n", 0xB8000);
    struct ppage video_tmp;
    video_tmp.next = NULL;
    video_tmp.prev = NULL;
    video_tmp.physical_addr = (void *)0xB8000;
    map_pages((void *)0xB8000, &video_tmp, pd);
    
    // Load page directory into CR3
    __asm__ __volatile__("mov %0, %%cr3" : : "r"(pd));
    
    // Enable paging by setting bit 31 and bit 0 of CR0
    __asm__ __volatile__(
        "mov %%cr0, %%eax\n"
        "or $0x80000001, %%eax\n"
        "mov %%eax, %%cr0"
        : : : "eax"
    );
    
    esp_printf(putc, "Paging enabled!\n");
}

struct page_directory_entry pd[1024] __attribute__((aligned(4096)));
struct page pt[1024] __attribute__((aligned(4096)));