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

/* shared buffer*/
int *buffer;

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

  while (1)
  {
    /* If all required items have been produced*/
    if (item_to_produce >= total_items)
    {
      break;
    }
    /* Adds item to buffer*/
    buffer[curr_buf_size++] = item_to_produce;
    print_produced(item_to_produce, thread_id);
    item_to_produce++;
  }
  return 0;
}

// write function to be run by worker threads
// ensure that the workers call the function print_consumed when they consume an item
void *consume_requests_loop(void *data){
  int thread_id = *((int *)data); /* Get thread ID from arguments*/

  while(1){
    /*If there are no times left to consume, break*/
    if(item_to_consume >= total_items){
      break;
    }
    if(curr_buf_size >0){ /*Is there something to consume*/
        int item = buffer[--curr_buf_size]; /* Remove item from buffer*/
        print_consumed(item, thread_id);
        item_to_consume++; 
    }
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

  return 0;
}
