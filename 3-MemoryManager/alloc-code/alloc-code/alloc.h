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
size_t free_list_head_size = 0;  // Global variable to store the size of the first block

/*declare pointer for memory page*/
static void *mem_page = NULL;
/*function declarations*/ 

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
    /*now to  initialize the free list with the total page size*/

    free_list = (MemoryBlock*)mem_page;
    free_list->size = PAGESIZE;
    free_list->is_free = true;
    free_list->next =  NULL;
    free_list->start = mem_page;

    
    printf("init_alloc: Successfully allocated %d bytes at %p\n", PAGESIZE, mem_page);
    print_memory_layout();
    return 0;  // Success
}


/*The function cleanup() must cleanup state of your manager, and return the
memory mapped page back to the OS. This function must return 0 on success
and a non-zero error code otherwise. */
int cleanup(){
    /*use munmap to free page pointer */
    if(mem_page == NULL){ /*if page pointer is null, indicate error?*/
        return 1;
    }
    if(munmap(mem_page, PAGESIZE)){ // if munmap fails, return 1
        return 1;
    }
    mem_page = NULL; // reset pointer after freeeing memory
    free_list= NULL;
    /*cleaning*/
    print_memory_layout();
    return 0; // Success
}


/*The function alloc(int) takes an integer bu_er size that must be allocated
and returns a char * pointer to the bu_er on a success. This function returns a
NULL on failure (e.g.,                                                                                                                                                                                                                                                                                                                                                                                                     requested size is not a multiple of 8 bytes, or insu_icient
free space). When successful, the returned pointer should point to a valid
memory address within the 4KB page of the memory manager*/
char *alloc(int size){
    printf("alloc: Requesting %d bytes\n", size);
    /*Error Handling*/
    /*Check if size is mulutple of 8  byes*/
    if(size % 8 !=0){
        printf("alloc: Invalid size request\n");
        return NULL;
    }
    /* What if size is 0 or greater than page size. Return Null*/
    if(size <=0 || size >PAGESIZE){
        printf("alloc: Invalid size request\n");
        return NULL;
    }
    /*Check for succficent free space . Seaarch free list for continous segments of memory that combined for suffirenct space*/
    
    MemoryBlock *prev = NULL;
    MemoryBlock *current = free_list;

    while (current) { /* Iterate through free list*/
        if (current->size >= size&& current->is_free==true) { /*Found a    block*/
            printf("alloc: Found block of size %ld at %p\n", current->size, current->start);
            /*Exact fit*/
            if(current->size==size){
                current->is_free = false;

                printf("alloc: Exact fit. Allocated %d bytes at %p\n", size, current->start);
                free_list_head_size = free_list->size;
                return current->start;
            }
            /* Splitting the larger block*/
            MemoryBlock *remaining = current->next; /*Temporarily store next free block*/
            MemoryBlock *new_free_block = (MemoryBlock *)((char *)current->start + size); /*Compute starting addresss of new block*/

            new_free_block->start = (char *)current->start + size; /*comm*/
            new_free_block->size = current->size - size;
            new_free_block->is_free = true;
            new_free_block->next = remaining;

            

             /*Update allocated block */
            current->size = size;
            current->is_free = false;
            /* Update free list*/
            current->next = new_free_block;

            printf("alloc: Split block. Allocated %d bytes at %p, remaining block at %p of size %ld\n",
                size, current->start, new_free_block->start, new_free_block->size);

           
            print_memory_layout();
            free_list_head_size = free_list->size;
            return current->start;
        }
        prev = current; /*Iterate*/
        current = current->next;
    }

    return NULL;/*If no suitable block found, return Null*/
}


/* Inserts a freed block into the free list in sorted order
ensures that the ree list remains sorted based on memory addresses*/



void merge_connecting_free_blocks(){
    MemoryBlock *current = free_list;

    /*Traceerse freee list, checking for adjacent blocks*/
    while (current && current->next) {
        if ((char *)current->start + current->size == current->next->start) { /*If two block are adjacent*/
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


/*The function dealloc(char *) takes a pointer to a previously allocated
memory chunk, and frees up the entire chunk.*/
void dealloc(char * ptr){
    free_list->start = mem_page;
    free_list->size = free_list_head_size;
    printf("freelist size", )
    printf("Dealloc iniated");
    print_memory_layout();

    if(!ptr) {
        printf("dealloc: Null pointer, ignoring\n");
        return;
    } /*Make sure ptr exists*/
    printf("dealloc: Attempting to free pointer at %p\n", ptr);

    /* Find the allocated block*/
    MemoryBlock *current = free_list;

    while (current) {
        printf("dealloc: Checking allocated block at %p, ptr is %p\n", current->start, ptr); // Added print statement
        if(current->start == ptr && current->is_free==false){
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


void print_memory_layout() {
    MemoryBlock *current;

    printf("\n===== MEMORY LAYOUT =====\n");
    
 
   

    // Print free blocks
    printf("Memory blocks:\n");
    current = free_list;
    while (current) {
        if (current->is_free) {
            printf("[F %p - %p | %zu bytes] -> ", 
                   current->start, 
                   (char *)current->start + current->size, 
                   current->size);
        } else {
            printf("[A %p - %p | %zu bytes] -> ", 
                   current->start, 
                   (char *)current->start + current->size, 
                   current->size);
        }
        current = current->next;
    }
    printf("NULL\n");
    
    printf("=========================\n\n");
}





