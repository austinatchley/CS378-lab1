#define _GNU_SOURCE

#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sched.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/sysinfo.h>

volatile int64_t *counter;
int64_t max_counter;
int64_t *inc_counters;

int64_t *load_difference;

void *worker_thread(void *idx);
void Create(pthread_t *thread, const pthread_attr_t *attr,
           void *(*start_routine) (void *), void *arg);
void Join(pthread_t thread, void **retval);
void timer_handler(void);
void do_counter(int num_workers);
void Set_affinity(pthread_t thread, size_t cpusetsize,
    const cpu_set_t *cpuset);

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

  int cores = get_nprocs();
  if(cores < 1)
    fprintf(stderr, "Error with cores");
  printf("Num cores: %d", get_nprocs());

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
  cpu_set_t cpuset;
  CPU_ZERO(&cpuset);

  // Create threads to execute worker_thread()
  for(int i = 0; i < num_workers; i++)
  {
    int *j = malloc(sizeof(int));
    if(j == NULL)
      exit(0);
    
    CPU_ZERO(&cpuset);
    CPU_SET(i % get_nprocs(), &cpuset);

    Set_affinity(threads[i], sizeof(cpu_set_t), &cpuset);

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
    (*counter)++;
    inc_counters[index]++;
  }
  return NULL;
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

void Set_affinity(pthread_t thread, size_t cpusetsize,
    const cpu_set_t *cpuset) {
  int result = pthread_setaffinity_np(thread, cpusetsize, cpuset);

  if(result)
    fprintf(stderr, "ERROR: Setting affinity");
}
