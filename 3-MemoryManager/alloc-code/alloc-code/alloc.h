#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

#define PAGESIZE 4096 /*size of memory to allocate from OS*/
#define MINALLOC 8 /*allocations will be 8 bytes or multiples of it*/

/*structure to store memory*/
typedef struct MemoryBlock
{
    void *start; // starting address of the block
    size_t size; // size of the memory block
    struct MemoryBlock *next;
    bool is_free;
    
} MemoryBlock;
//
MemoryBlock *free_list =NULL;
MemoryBlock *allocated_list = NULL;
/*declare pointer for memory page*/
static void *mem_page = NULL;


/*The function init alloc() must initialize the memory manager, including
allocating a 4KB page from the OS via mmap, and initializing any other data
structures required. This function will be invoked by the user before requesting
any memory from your memory manager. This function must return 0 on
success and a non-zero error code otherwise. */
int init_alloc() {
    /*using mmap to allocate a memory page of pages size*/
    mem_page = mmap(NULL, PAGESIZE, PROT_READ | PROT_WRITE,
                    MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    if (mem_page == MAP_FAILED) {
        printf("Error: mmap failed\n");
        return 1;  /* Return 1 on failure (as expected by test code)*/
    }

    /*now to initialize the free list with the total page size*/
    free_list = (MemoryBlock*)mem_page;
    free_list->size = PAGESIZE;
    free_list->is_free = true;
    free_list->next = NULL;
    free_list->start = mem_page;

    /*Initialize the allocated_list to null*/
    allocated_list = NULL;

    printf("init_alloc: Successfully allocated %d bytes at %p\n", PAGESIZE, mem_page);
    return 0;  // Success
}

/*The function alloc(int) takes an integer buffer size that must be allocated
and returns a char * pointer to the buffer on a success. This function returns a
NULL on failure (e.g., requested size is not a multiple of 8 bytes, or insufficient
free space). When successful, the returned pointer should point to a valid
memory address within the 4KB page of the memory manager*/
char *alloc(int size) {
    printf("alloc: Requesting %d bytes\n", size);

    /*Error Handling*/
    /*Check if size is multiple of 8 bytes*/
    if (size % 8 != 0) {
        printf("alloc: Invalid size request - must be multiple of 8\n");
        return NULL;
    }
    /* What if size is 0 or greater than page size. Return Null*/
    if (size <= 0 || size > PAGESIZE) {
        printf("alloc: Invalid size request - out of bounds\n");
        return NULL;
    }

    /*Check for sufficient free space. Search free list for contiguous segments of memory that combined for sufficient space*/
    MemoryBlock *prev = NULL;
    MemoryBlock *current = free_list;

    while (current) { /* Iterate through free list*/
        if (current->size >= size) { /*Found a block*/
            printf("alloc: Found block of size %ld at %p\n", current->size, current->start);

            /*Exact fit*/
            if (current->size == size) {
                current->is_free = false;

                /*Remove from free list*/
                if (prev) { /*If not at front of the list*/
                    prev->next = current->next;
                } else { /*If first node in list*/
                    free_list = current->next;
                }

                /* Move to allocated list*/
                current->next = allocated_list;
                allocated_list = current;

                printf("alloc: Exact fit. Allocated %d bytes at %p\n", size, current->start);
                return current->start;
            }

            /* Splitting the larger block*/
            MemoryBlock *remaining = current->next; /*Temporarily store next free block*/
            MemoryBlock *new_free_block = (MemoryBlock *)((char *)current->start + size); /*Compute starting address of new block*/

            new_free_block->start = (char *)current->start + size; /*Compute new free block start address*/
            new_free_block->size = current->size - size;
            new_free_block->is_free = true;
            new_free_block->next = remaining;

            /* Update free list*/
            if (prev) prev->next = new_free_block;
            else free_list = new_free_block;

            /*Update allocated block */
            current->size = size;
            current->is_free = false;

            /*Move allocated block to allocated list*/
            current->next = allocated_list;
            allocated_list = current;

            printf("alloc: Split block. Allocated %d bytes at %p, remaining block at %p of size %ld\n",
                   size, current->start, new_free_block->start, new_free_block->size);
            return current->start;
        }
        prev = current; /*Iterate*/
        current = current->next;
    }

    printf("alloc: No suitable block found\n");
    return NULL; /*If no suitable block found, return Null*/
}

/*The function dealloc(char *) takes a pointer to a previously allocated
memory chunk, and frees up the entire chunk.*/
void dealloc(char *ptr) {
    if (!ptr) {
        printf("dealloc: Null pointer, ignoring\n");
        return; /*Make sure ptr exists*/
    }

    /* Find the allocated block*/
    MemoryBlock *prev = NULL;
    MemoryBlock *current = allocated_list;

    while (current && current->start != ptr) {
        prev = current;
        current = current->next;
    }

    if (!current) {
        printf("dealloc: Pointer not found in allocated list\n");
        return; /*If ptr isn't in allocated list, return*/
    }

    printf("dealloc: Freeing memory at %p of size %ld\n", current->start, current->size);

    /*Remove from allocated list*/
    if (prev) {
        prev->next = current->next;
    } else {
        allocated_list = current->next;
    }

    current->is_free = true; /*Mark as free*/

    /*now to insert back to into free list, sorted*/
    insert_into_freelist(current);

    /*Merge any blocks that are adjacent*/
    merge_connecting_free_blocks();
}

/* Inserts a freed block into the free list in sorted order
ensures that the free list remains sorted based on memory addresses*/
void insert_into_freelist(MemoryBlock *block) {
    /*If freelist empty or block should be at beginning*/
    if (!free_list || block->start < free_list->start) {
        block->next = free_list; /*Insert block at the start*/
        free_list = block;
        printf("insert_into_freelist: Inserted block at start, address %p\n", block->start);
        return;
    }
    MemoryBlock *prev = free_list;
    MemoryBlock *current = free_list->next;

    /*Traverse free list to find corresponding insertion point*/
    while (current && block->start > current->start) {
        prev = current;
        current = current->next;
    }

    /*Insert block between previous and current*/
    block->next = current;
    prev->next = block;

    printf("insert_into_freelist: Inserted block at address %p between %p and %p\n",
           block->start, prev->start, current ? current->start : NULL);
}

/* Merge adjacent free blocks */
void merge_connecting_free_blocks() {
    MemoryBlock *current = free_list;

    /*Traverse free list, checking for adjacent blocks*/
    while (current && current->next) {
        if ((char *)current->start + current->size == current->next->start) { /*If two blocks are adjacent*/
            /*merge them*/
            printf("merge_connecting_free_blocks: Merging blocks at %p and %p\n",
                   current->start, current->next->start);
            current->size += current->next->size;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}





