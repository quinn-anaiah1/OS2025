#ifndef ALLOC_H
#define ALLOC_H

#include <stdio.h>
#include <stdbool.h>
#include <sys/mman.h>

#define PAGESIZE 4096  // Size of memory page (4KB)
#define MINALLOC 8      // Minimum allocation size
#define MAX_BLOCKS 512  // Max metadata blocks to track allocations

/* Structure to store memory metadata */
typedef struct MemoryBlock {
    void *start;            // Starting address of the block
    size_t size;            // Size of the memory block
    struct MemoryBlock *next;
    bool is_free;           // Indicates if the block is free
} MemoryBlock;

/* Global variables */
static MemoryBlock metadata[MAX_BLOCKS];  // Metadata stored externally
static int metadata_count = 0;            // Number of active metadata blocks
static MemoryBlock *free_list = NULL;     // Head of the free list
static void *mem_page = NULL;             // Pointer to allocated memory page

/* Function declarations */
int init_alloc();
int cleanup();
char *alloc(int size);
void dealloc(char *ptr);
void print_memory_layout();
void merge_connecting_free_blocks();

/* Initializes the memory manager */
int init_alloc() {
    mem_page = mmap(NULL, PAGESIZE, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);
    if (mem_page == MAP_FAILED) {
        printf("Error: mmap failed\n");
        return 1;
    }

    // Initialize free list using statically allocated metadata array
    free_list = &metadata[0];
    free_list->start = mem_page;
    free_list->size = PAGESIZE;
    free_list->is_free = true;
    free_list->next = NULL;

    metadata_count = 1;  // One block in use
    printf("init_alloc: Successfully allocated %d bytes at %p\n", PAGESIZE, mem_page);
    print_memory_layout();
    return 0;
}

/* Cleans up memory and releases back to the OS */
int cleanup() {
    if (mem_page == NULL) return 1;
    if (munmap(mem_page, PAGESIZE)) return 1;

    // Reset global variables
    mem_page = NULL;
    free_list = NULL;
    metadata_count = 0;
    printf("cleanup: Memory released\n");
    return 0;
}

/* Allocates memory of given size */
char *alloc(int size) {
    printf("alloc: Requesting %d bytes\n", size);

    // Check if size is valid (multiple of 8, within limits)
    if (size % 8 != 0 || size <= 0 || size > PAGESIZE) {
        printf("alloc: Invalid size request\n");
        return NULL;
    }

    // Search free list for a suitable block
    MemoryBlock *current = free_list;
    while (current) {
        if (current->is_free && current->size >= size) {
            printf("alloc: Found block of size %ld at %p\n", current->size, current->start);

            if (current->size > size) { // Split block
                if (metadata_count >= MAX_BLOCKS) return NULL;  // No space for metadata
                MemoryBlock *new_block = &metadata[metadata_count++];

                new_block->start = (char *)current->start + size;
                new_block->size = current->size - size;
                new_block->is_free = true;
                new_block->next = current->next;

                current->size = size;
                current->is_free = false;
                current->next = new_block;

                printf("alloc: Split block. Allocated %d bytes at %p, remaining %ld bytes at %p\n",
                       size, current->start, new_block->size, new_block->start);
            } else {
                current->is_free = false;
                printf("alloc: Exact fit. Allocated %d bytes at %p\n", size, current->start);
            }
            print_memory_layout();
            return (char *)current->start;
        }
        current = current->next;
    }
    printf("alloc: No suitable block found\n");
    return NULL;
}

/* Frees allocated memory */
void dealloc(char *ptr) {
    if (!ptr) {
        printf("dealloc: Null pointer, ignoring\n");
        return;
    }

    printf("dealloc: Attempting to free pointer at %p\n", ptr);
    MemoryBlock *current = free_list;

    while (current) {
        if (current->start == ptr && !current->is_free) {
            printf("dealloc: Found block at %p, freeing it\n", ptr);
            current->is_free = true;
            merge_connecting_free_blocks();
            print_memory_layout();
            return;
        }
        current = current->next;
    }

    printf("dealloc: Pointer not found in allocated list\n");
}

/* Merges adjacent free blocks */
void merge_connecting_free_blocks() {
    MemoryBlock *current = free_list;
    while (current && current->next) {
        if (current->is_free && current->next->is_free &&
            (char *)current->start + current->size == current->next->start) {
            printf("merge: Merging blocks at %p and %p\n", current->start, current->next->start);
            current->size += current->next->size;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}

/* Prints the current memory layout */
void print_memory_layout() {
    MemoryBlock *current = free_list;
    printf("\n===== MEMORY LAYOUT =====\n");
    while (current) {
        printf("[%c %p - %p | %zu bytes] -> ",
               current->is_free ? 'F' : 'A',
               current->start,
               (char *)current->start + current->size,
               current->size);
        current = current->next;
    }
    printf("NULL\n=========================\n\n");
}

#endif
