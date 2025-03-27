#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int NUM_THREADS;
int current_thread = 0;  // Keeps track of whose turn it is
pthread_mutex_t lock;

void *myThreadFun(void *threadNum) {
    int num = *(int *)threadNum; // Copy thread number

    while (1) {
        pthread_mutex_lock(&lock);

        // Only print if it's this thread's turn
        while (current_thread != num) {
            pthread_mutex_unlock(&lock);
            usleep(1000);  // Small delay to prevent busy waiting
            pthread_mutex_lock(&lock);
        }

        printf("Printing Thread Num: %d\n", num);
        
        // Move to the next thread
        current_thread = (current_thread + 1) % NUM_THREADS;

        pthread_mutex_unlock(&lock);
        sleep(1);
    }

    return NULL;
}

int main() {
    printf("Enter the number of threads: ");
    if (scanf("%d", &NUM_THREADS) != 1 || NUM_THREADS <= 0) {
        printf("Invalid input. Please enter a valid integer.\n");
        return 1;
    }

    printf("Number of threads: %d\n", NUM_THREADS);
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    pthread_mutex_init(&lock, NULL);

    // Create threads
    for (int i = 0; i < NUM_THREADS; i++) {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, myThreadFun, &thread_ids[i]) != 0) {
            perror("Failed to create thread");
            return 1;
        }
    }

    // Join threads (will never be reached due to infinite loop)
    for (int i = 0; i < NUM_THREADS; i++) {
        pthread_join(threads[i], NULL);
    }

    pthread_mutex_destroy(&lock);
    return 0;
}
