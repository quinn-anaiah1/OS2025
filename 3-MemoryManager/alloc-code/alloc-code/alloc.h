#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>

#define PAGESIZE 4096 //size of memory to allocate from OS
#define MINALLOC 8 //allocations will be 8 bytes or multiples of it

//structure to store memory
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
//declare pointer for memory page
static void *mem_page = NULL;
// function declarations

/*The function init alloc() must initialize the memory manager, including
allocating a 4KB page from the OS via mmap, and initializing any other data
structures required. This function will be invoked by the user before requesting
any memory from your memory manager. This function must return 0 on
success and a non-zero error code otherwise. */



int init_alloc() {
    // using mmap to allocate a memory page of pages size
    mem_page = mmap(NULL, PAGESIZE, PROT_READ | PROT_WRITE,
                       MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    if (mem_page == MAP_FAILED) {
        return 1;  // Return 1 on failure (as expected by test code)
    }
    //now to  initialize the free list with the total page size

    free_list = (MemoryBlock*)mem_page;
    free_list->size = PAGESIZE;
    free_list->is_free = true;
    free_list->next =  NULL;
    free_list->start = mem_page;

    //Initialize the allocated_list to null
    allocated_list = NULL;

    return 0;  // Success
}


/*The function cleanup() must cleanup state of your manager, and return the
memory mapped page back to the OS. This function must return 0 on success
and a non-zero error code otherwise. */
int cleanup(){
    //use munmap to free page pointer 
    if(mem_page == NULL){ //if page pointer is null, indicate error?
        return 1;
    }
    if(munmap(mem_page, PAGESIZE)){ // if munmap fails, return 1
        return 1;
    }
    mem_page = NULL // reset pointer after freeeing memory

    //optional cleaning the linked list
    return 0; // Success
}


/*The function alloc(int) takes an integer bu_er size that must be allocated
and returns a char * pointer to the bu_er on a success. This function returns a
NULL on failure (e.g., requested size is not a multiple of 8 bytes, or insu_icient
free space). When successful, the returned pointer should point to a valid
memory address within the 4KB page of the memory manager*/
char *alloc(int);


/*The function dealloc(char *) takes a pointer to a previously allocated
memory chunk, and frees up the entire chunk.*/
void dealloc(char *);
