/**
 * @file T0_Test.c
 * @brief A simple C program that runs for a long duration by pausing for user input.
 * @author Anaiah Quinn
 * @date March 16 2025
 */
#include <stdio.h>
#include <unistd.h>

int main() {
    printf("Process ID: %d\n", getpid());
    printf("Sleeping... Press Enter to exit.\n");

    getchar();  // Wait for user input

    return 0;
}
