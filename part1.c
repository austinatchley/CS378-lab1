#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>

int *counter;
int max_counter;

void *worker_thread(void *UNUSED);
void Create(pthread_t *thread, const pthread_attr_t *attr,
           void *(*start_routine) (void *), void *arg);
void Join(pthread_t thread, void **retval);
void timer_handler(void);
void do_counter(int num_workers);

int main(int argc, char** argv)
{
  if(argc < 2)
    printf("lab1 --counter --workers (optional: --iterations)");

  counter = malloc(sizeof(int));
  *counter = 0;
 
  max_counter = atoi(argv[1]);
  printf("Max counter: %d\n", max_counter);
 
  int num_workers = atoi(argv[2]);
  printf("Num workers: %d\n", num_workers);

  int iterations = 1;
  if(argc > 3)
    iterations = atoi(argv[3]);

  printf("Num iterations: %d\n", iterations);
  
  struct timeval start, stop;
  gettimeofday(&start, NULL);
  
  for(int i = 0; i < iterations; i++)
    do_counter(num_workers);

  gettimeofday(&stop, NULL);

  double diff = stop.tv_usec - start.tv_usec;
  double avg = diff / ((double) iterations);

  printf("Total Time: %f microseconds\nAverage: %f microseconds\n",
         diff, avg);
}

void do_counter(int num_workers)
{ 
  // Create an array to store thread ptrs
  pthread_t *threads = malloc(num_workers * sizeof(pthread_t));

  // Create threads to execute worker_thread()
  for(int i = 0; i < num_workers; i++)
  {
    Create(&threads[i], NULL, worker_thread, NULL);
  }

  // Join threads
  for(int i = 0; i < num_workers; i++)
  {
    Join(threads[i], NULL);
  }
}

void *worker_thread(void *UNUSED)
{
  int my_increment_count = 0;
  while(*counter < max_counter)
  {
    (*counter)++;
    my_increment_count++;
  }
  // printf("My increment counter: %d\n", my_increment_count);
}

void Create(pthread_t *thread, const pthread_attr_t *attr,
           void *(*start_routine) (void *), void *arg)
{
  int result = pthread_create(thread, attr, start_routine, arg);
  
  if(result)
    //TODO: ERROR HANDLING
    fprintf(stderr, "ERROR: Creating thread\n");
}

void Join(pthread_t thread, void **retval)
{
  int result = pthread_join(thread, retval);
  
  if(result)
    //TODO: ERROR HANDLING
    fprintf(stderr, "ERROR: Joining thread\n");
}
