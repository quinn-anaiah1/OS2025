/**
 * @file T0_Test.c
 * @brief A simple C program that runs for a long duration by pausing for user input.
 * Augnmented to o memory map an empty page from the OS
 * it makes sense to ask the OS for an anonymous
 * page (since it is not backed by any file on disk) and in private mode (since you are not
 * sharing this page with other processes)
 * @author Anaiah Quinn
 * @date March 16 2025
 * @version 1.1 Requesting an em
 */
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/mman.h>

int main() {
    printf("Process ID: %d\n", getpid());
    

    //requesting page of standard size from the OS
    void *mapped_mem_pt = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_PRIVATE, -1, 0);

    //error handdling
    if(mapped_mem_pt == MAP_FAILED){
        perror("mmap failed")
        exit(1)
    }
    printf("Memory mapped successfull. Now Sleeping.. Press Enter to exit");
    getchar();  // Wait for user input

    
    //free memory before exit
    munmap(mapped_mem_pt, 4096)
    return 0;
}
