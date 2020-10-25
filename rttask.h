#ifndef _RTTASK_H_
#define _RTTASK_H_

#include <time.h>
#include <pthread.h>

// Time managment functions
void time_copy(struct timespec* des,
               const struct timespec* src);
void time_add_ms(struct timespec* time, int msec);
int time_cmp(const struct timespec* t1,
             const struct timespec* t2);

// Task functions
struct task_info {
  pthread_t thread_id;
  int task_num;
  void* arg;
  long wcet_ms;
  int period_ms;
  int deadline_ms;
  int priority;
  int deadline_miss;
  struct timespec next_activation;
  struct timespec abs_deadline;
};

void task_info_init(struct task_info* task, int task_num,
                    int period_ms, int deadline_ms,
                    int priority);
int task_deadline_missed(struct task_info* task);
void task_set_period(struct task_info* task);
void task_wait_for_period(struct task_info* task);

int task_create(struct task_info* task, void* (*func)(void*));
#endif