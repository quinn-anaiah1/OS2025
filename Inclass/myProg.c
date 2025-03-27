#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> //Header file for sleep(). man 3 sleep for details.

int NUM_THREADS; // global variable storing number of threads
pthread_mutex_t lock;

// A normal C function that is executed as a thread
// when its name is specified in pthread_create()
void *myThreadFun(void *threadNum)
{

    while (1 == 1)
    {
        pthread_mutex_lock(&lock);
        int num = *(int *)threadNum;
        printf("Printing Thread Num: %d \n", num);
        pthread_mutex_unlock(&lock);
        sleep(1);
    }

    
    return NULL;
}

// each thread should print its own thread id
int main()
{
    int N;
    // get the number of threads to create from user
    printf("Enter the number of threads: ");
    if (scanf("%d", &N) != 1)
    {
        printf("Invalid input. Please enter a valid integer.\n");
        return 1; // Return an error code
    }

    printf("Number of thread: %d \n", N);
    NUM_THREADS = N;
    pthread_t threads[NUM_THREADS];
    int thread_ids[NUM_THREADS];

    int i = 0;

    /*Create N threads*/
    for (int i = 0; i < NUM_THREADS; i++)
    {
        thread_ids[i] = i;
        if (pthread_create(&threads[i], NULL, myThreadFun, &thread_ids[i]) != 0)
        {
            perror("Failed to create thread");
            return 1;
        }
    }
    for (int i = 0; i < NUM_THREADS; i++)
    {
        pthread_join(threads[i], NULL);
    }

    /*Joing N threads*/

    printf("Before Thread\n");

    printf("After Thread\n");
    exit(0);
}
