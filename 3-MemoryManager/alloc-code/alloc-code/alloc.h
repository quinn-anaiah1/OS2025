#include <stdio.h>
#include <string.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdlib.h>

#define PAGESIZE 4096 //size of memory to allocate from OS
#define MINALLOC 8 //allocations will be 8 bytes or multiples of it

// function declarations

/*The function init alloc() must initialize the memory manager, including
allocating a 4KB page from the OS via mmap, and initializing any other data
structures required. This function will be invoked by the user before requesting
any memory from your memory manager. This function must return 0 on
success and a non-zero error code otherwise. */
int init_alloc(); \


/*The function cleanup() must cleanup state of your manager, and return the
memory mapped page back to the OS. This function must return 0 on success
and a non-zero error code otherwise. */
int cleanup(); 


/*The function alloc(int) takes an integer bu_er size that must be allocated
and returns a char * pointer to the bu_er on a success. This function returns a
NULL on failure (e.g., requested size is not a multiple of 8 bytes, or insu_icient
free space). When successful, the returned pointer should point to a valid
memory address within the 4KB page of the memory manager*/
char *alloc(int);


/*The function dealloc(char *) takes a pointer to a previously allocated
memory chunk, and frees up the entire chunk.*/
void dealloc(char *);
