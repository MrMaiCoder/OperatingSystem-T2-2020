/**
 * threadpool.c
 *
 * This file will contain your implementation of a threadpool.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "threadpool.h"

typedef struct _job_st{  
  struct _job_st* next;                
	void   (*function)(void*);       // function pointer          
	void*  arg;                      // function's argument       
} job;

// _threadpool is the internal threadpool structure that is
// cast to type "threadpool" before it given out to callers
typedef struct _threadpool_st {
  pthread_t* threads;
  int job_num;
  job* head;
  job* tail;
  pthread_mutex_t thread_lock;
  pthread_cond_t thread_cond;
} _threadpool;



void *worker_thread(threadpool thread_pool) {
  _threadpool *pool = (_threadpool*) thread_pool;
  job* job_p;
    while (1) {
      
      pool->job_num = pool->job_num;
      pthread_mutex_lock(&pool->thread_lock);
      // if no job, wait for signal
      while(pool->job_num == 0){
        pthread_mutex_unlock(&(pool->thread_lock));
        pthread_cond_wait(&pool->thread_cond, &pool->thread_lock);
      }
      // get the job and run it
      job_p = pool->head;
       
      // decrement job number
      pool->job_num--;

       // set next job to head
      if(pool->job_num == 0){
        pool->head = NULL;
        pool->tail = NULL;
      }else{
        pool->head = job_p->next;
      }
      pthread_mutex_unlock(&pool->thread_lock);
      (job_p->function)(job_p->arg);
      free(job_p);
    }
}


threadpool create_threadpool(int num_threads_in_pool) {
  _threadpool *pool;

  // sanity check the argument
  if ((num_threads_in_pool <= 0) || (num_threads_in_pool > MAXT_IN_POOL))
    return NULL;

  pool = (_threadpool *) malloc(sizeof(_threadpool));
  if (pool == NULL) {
    fprintf(stderr, "Out of memory creating a new threadpool!\n");
    return NULL;
  }

  // add your code here to initialize the newly created threadpool

  // initialize the thread pool
  pool->threads = (pthread_t *)malloc(sizeof(pthread_t) * num_threads_in_pool);
  pool->job_num = 0;
  pool->head = NULL;
  pool->tail = NULL;

  pthread_mutex_init(&(pool->thread_lock), NULL);
  pthread_cond_init(&(pool->thread_cond), NULL);

  for (int i = 0; i < num_threads_in_pool; i++)
  {
    pthread_create(pool->threads + i, NULL, worker_thread, pool);
  }

  return (threadpool) pool;
}


void dispatch(threadpool from_me, dispatch_fn dispatch_to_here, void *arg) {
  _threadpool *pool = (_threadpool *) from_me;

// create a new job
  job* newjob = (job*)malloc(sizeof(job));

  newjob->function = dispatch_to_here;
  newjob->arg = arg;
  newjob->next = NULL;

  pthread_mutex_lock(&pool->thread_lock);

  // add the new job into queue
  if (pool->job_num == 0){
    pool->head = newjob;
    pool->tail = newjob;
    pthread_cond_signal(&pool->thread_cond);
  }else{
    pool->tail->next = newjob;
    pool->tail = newjob;
  }
  pool->job_num++;
  pthread_mutex_unlock(&pool->thread_lock);
}

void destroy_threadpool(threadpool destroyme) {
  _threadpool *pool = (_threadpool *) destroyme;

  // add your code here to kill a threadpool
free(pool->threads);
pthread_mutex_destroy(&(pool)->thread_lock);
pthread_cond_destroy(&(pool)->thread_cond);
  
}


