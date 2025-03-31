#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <wait.h>
#include <pthread.h>

/* Global variables for tracking production and consumption*/
int item_to_produce, item_to_consume, curr_buf_size;
int total_items, max_buf_size, num_workers, num_masters;
int done_producing = 0; /*A flag to indicate when all producers are done*/

/* shared buffer*/
int *buffer;

/*Synchronization variables*/
pthread_mutex_t buffer_mutex = PTHREAD_MUTEX_INITIALIZER; /*Protecting access to shared buffer*/
pthread_cond_t buffer_not_full =PTHREAD_COND_INITIALIZER; /*Signals producers that buffer isnt full*/
pthread_cond_t buffer_not_empty = PTHREAD_COND_INITIALIZER; /*Signals consumers that buffer isnt empty*/

/* Function to prin a produced item*/
void print_produced(int num, int master)
{

  printf("Produced %d by master %d\n", num, master);
}

/* Function to print a consumed*/
void print_consumed(int num, int worker)
{

  printf("Consumed %d by worker %d\n", num, worker);
}

/*Function that generates/produces items and place in buffer*/
/*modify code below to synchronize correctly*/

void *generate_requests_loop(void *data)
{
  int thread_id = *((int *)data); /* Get thread ID from arguments*/

  while (1){
    pthread_mutex_lock(&buffer_mutex);/*Lock buffer mutex*/


    while(curr_buf_size == max_buf_size){ /* Wait if the buffer is currently full*/
      pthread_cond_wait(&buffer_not_full, &buffer_mutex);/* Wait for the signal that the buffer isnt full*/
    }
    /* If all required items have been produced, exit loop*/
    if (item_to_produce >= total_items){
      done_producing = 1; /* Indicate the productions is complete*/
      pthread_cond_broadcast(&buffer_not_empty); /*Wake all wating worker threads*/
      pthread_mutex_unlock(&buffer_mutex); /* Unlock before exiting*/
      break;
    }
    /* Adds item to buffer*/
    buffer[curr_buf_size++] = item_to_produce;
    print_produced(item_to_produce, thread_id);
    item_to_produce++;

    pthread_cond_signal(&buffer_not_empty);/*Signal that the buffer isnt empty*/
    pthread_mutex_unlock(&buffer_mutex); /* Unlock so other threads can access the buffer*/
  }
  return 0;
}


// write function to be run by worker threads
// ensure that the workers call the function print_consumed when they consume an item
void *consume_requests_loop(void *data){
  int thread_id = *((int *)data); /* Get thread ID from arguments*/

  while(1){

    pthread_mutex_lock(&buffer_mutex); /* Lock access to the shared buffer */
    
    while(curr_buf_size == 0 && !done_producing){ /* If buffer is empty and producers still running, wait*/
      pthread_cond_wait(&buffer_not_empty, &buffer_mutex);
    }
    
    /*If there are no times left to consume, break*/
    if(item_to_consume >= total_items){
      pthread_mutex_unlock(&buffer_mutex);
      break;
    }
    
    /*Remove item from buffer*/
    int item = buffer[--curr_buf_size]; /* Remove item from buffer*/
    print_consumed(item, thread_id);
    item_to_consume++; 

    /* Signal that the buffer now has space*/
    pthread_cond_signal(&buffer_not_full);
    pthread_mutex_unlock(&buffer_mutex);
    }
   return 0;
}

int main(int argc, char *argv[])
{
  int *master_thread_id;
  pthread_t *master_thread;
  item_to_produce = 0;
  curr_buf_size = 0;

  /*Initializing worker threads*/
  int *worker_thread_id;
  pthread_t * worker_thread;
  item_to_consume = 0;
  

  int i;

  if (argc < 5)/*Validate input*/
  {
    printf("./master-worker #total_items #max_buf_size #num_workers #masters e.g. ./exe 10000 1000 4 3\n");
    exit(1);
  }
  else
  { /* Parsing command line args*/
    num_masters = atoi(argv[4]);
    num_workers = atoi(argv[3]);
    total_items = atoi(argv[1]);
    max_buf_size = atoi(argv[2]);
  }

  /*Allocating mememory for the shared buffer*/
  buffer = (int *)malloc(sizeof(int) * max_buf_size);

  
  /* Allocating memory for the master thread IDs and thread objects*/
  master_thread_id = (int *)malloc(sizeof(int) * num_masters);
  master_thread = (pthread_t *)malloc(sizeof(pthread_t) * num_masters);
  
  /* Assign thread ids*/
  for (i = 0; i < num_masters; i++)
    master_thread_id[i] = i;

  /*Creating master producer threads*/
  for (i = 0; i < num_masters; i++)
    pthread_create(&master_thread[i], NULL, generate_requests_loop, (void *)&master_thread_id[i]);

  /* Allocating memory for the worker thread IDs and thread objects*/
  worker_thread_id = (int *)malloc(sizeof(int) * num_workers);
  worker_thread = (pthread_t *)malloc(sizeof(pthread_t) * num_workers);

  /* Assinging thread ids*/
  for( i =0; i<num_workers; i++)
    worker_thread_id[i] = i;

  /*Creating worker consumer threads */
  for (i = 0; i < num_workers; i++)
    pthread_create(&worker_thread[i], NULL, consume_requests_loop, (void *)&worker_thread_id[i]);


  /* Wating for all  master threads to complete exection*/
  for (i = 0; i < num_masters; i++)
  {
    pthread_join(master_thread[i], NULL);
    printf("master %d joined\n", i);
  }

  /* Wating for all worker threads to complete exection*/
  for (i = 0; i < num_workers; i++)
  {
    pthread_join(worker_thread[i], NULL);
    printf("worker %d joined\n", i);
  }

  /*----Deallocating Buffers---------------------*/
  free(buffer);
  free(master_thread_id);
  free(master_thread);

  free(worker_thread_id);
  free(worker_thread);
  

  return 0;
}
