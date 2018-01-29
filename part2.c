#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>

volatile int64_t *counter;
int64_t max_counter;
int64_t *inc_counters;

int64_t *load_difference;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

void *worker_thread(void *idx);
void Create(pthread_t *thread, const pthread_attr_t *attr,
           void *(*start_routine) (void *), void *arg);
void Join(pthread_t thread, void **retval);
void timer_handler(void);
void do_counter(int num_workers);

int main(int argc, char** argv)
{
  if(argc < 3)
  {
    printf("lab1 --counter --workers --optional:iterations\n");
    return 1;
  }

  counter = malloc(sizeof(int));
  *counter = 0;
 
  max_counter = atoi(argv[1]);
  printf("Max counter: %li\n", max_counter);
 
  int num_workers = atoi(argv[2]);
  printf("Num workers: %d\n", num_workers);
  inc_counters = malloc(num_workers * sizeof(int64_t));

  int iterations = 1;
  if(argc > 3)
    iterations = atoi(argv[3]);

  printf("Num iterations: %d\n\n", iterations);

  load_difference = malloc(num_workers * sizeof(int64_t));

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
    int *j = malloc(sizeof(int));
    if(j == NULL)
      exit(0);
    *j=i;
    Create(&threads[i], NULL, worker_thread, (void *) j);
  }

  double sum = 0;

  // Join threads
  for(int i = 0; i < num_workers; i++)
  {
    Join(threads[i], NULL);
    printf("Increments %d:\t%li\n", i, inc_counters[i]);
    sum += inc_counters[i];
    load_difference[i] = abs((max_counter / num_workers) - inc_counters[i]);
  }

  double average_lost_ratio = sum / ((double)max_counter);
  printf("Ratio of lost updates:\t\t%f\n", average_lost_ratio);

  int64_t load_diff_sum = 0;
  for(int i = 0; i < num_workers; i++)
    load_diff_sum += load_difference[i];

  double average_load_diff = ((double) load_diff_sum  / (double) num_workers);
  printf("Average load difference:\t%f\n", average_load_diff);
}

void *worker_thread(void *idx)
{
  int index = *((int *) idx);
  while(*counter < max_counter)
  {
    pthread_mutex_lock(&lock);
    (*counter)++;
    inc_counters[index]++;
    pthread_mutex_unlock(&lock);
  }
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
