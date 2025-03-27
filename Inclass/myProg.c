#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> /*Header file for sleep(). man 3 sleep for details.*/

int NUM_THREADS; /* global variable storing number of threads*/
pthread_mutex_t lock; /*initializeing the mutex lock*/
pthread_cond_t cond; /*Conditiona vairbale to control execution order*/
int curr_thread = 0; /*Needed to keep track of thread print order*/

/* A normal C function that is executed as a thread*/
/* when its name is specified in pthread_create()*/
void *myThreadFun(void *threadNum)
{
    int num = *(int *)threadNum;/*Get thread number*/
    while (1) /*Infinite loop*/
    {
        pthread_mutex_lock(&lock);

        /*Wait until the current thread is num, waiting its term*/
        while (curr_thread!= num)
        {
            pthread_cond_wait(&cond, &lock); /*Put thread to sleep until its his turn*/
        }
        
        /* Now print the thread number*/
        printf("Printing Thread Num: %d \n", num);

        /* Update the curr_thread*/
        if(curr_thread == NUM_THREADS-1){
            curr_thread = 0;
        }else{
            curr_thread +=1;
        }
        
        /*Broadcast to wake all threads */
        pthread_cond_broadcast(&cond);

        pthread_mutex_unlock(&lock); /*unlock mutex*/        
        sleep(1);
    }

    
    return NULL;
}

/* each thread should print its own thread id*/
int main()
{
    int N;
    /*get the number of threads to create from user*/ 
    printf("Enter the number of threads: ");
    if (scanf("%d", &N) != 1)
    {
        printf("Invalid input. Please enter a valid integer.\n");
        return 1; /* Return an error code*/
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
