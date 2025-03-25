#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> //Header file for sleep(). man 3 sleep for details.
 
 
// A normal C function that is executed as a thread
// when its name is specified in pthread_create()
void* myThreadFun(void* threadNum)
{   
    //int num = (int *) threadNum;
    printf("Printing Thread Num: %s \n", (char*) threadNum);
    sleep(1);
    return NULL;
}
 
 // each thread should print its own thread id 
int main()
{
    pthread_t thread_id;
    int i= 0; 
    int N = getchar(); //ask user for how many threads
    printf("Number of threads\n", N);
    printf("Before Thread\n");
    pthread_create(&thread_id, NULL, myThreadFun, "0");
    pthread_join(thread_id, NULL);
    printf("After Thread\n");
    exit(0);
}
 