#include "rttask.h"

void time_copy(struct timespec* des, const struct timespec* src) {
  des->tv_sec = src->tv_sec;
  des->tv_nsec = src->tv_nsec;
}

void time_add_ms(struct timespec* time, int msec) {
  time->tv_sec += msec/1000;
  time->tv_nsec += (msec % 1000) * 1e6;

  if (time->tv_nsec > 1e9) {
    time->tv_nsec -= 1e9;
    time->tv_sec += 1;
  }
}

int time_cmp(const struct timespec* t1, const struct timespec* t2) {
  if (t1->tv_sec > t2->tv_sec) return 1;
  if (t1->tv_sec < t2->tv_sec) return -1;
  if (t1->tv_nsec > t2->tv_nsec) return 1;
  if (t1->tv_nsec < t2->tv_nsec) return -1;
  return 0;
}

// Task functions
void task_info_init(struct task_info* task, int task_num,
                    int period_ms, int deadline_ms,
                    int priority) {
  task->arg = NULL;
  task->task_num = task_num;
  task->wcet_ms = 0;
  task->period_ms = period_ms;
  task->deadline_ms = deadline_ms;
  task->priority = priority;
  task->deadline_miss = 0;
}

int task_deadline_missed(struct task_info* task) {
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  if (time_cmp(&now, &task->abs_deadline) > 0) {
    ++(task->deadline_miss);
    return 1;
  }

  return 0;
}

void task_set_period(struct task_info* task) {
  struct timespec now;
  clock_gettime(CLOCK_MONOTONIC, &now);
  time_copy(&task->next_activation, &now);
  time_add_ms(&task->next_activation, task->period_ms);

  time_copy(&task->abs_deadline, &now);
  time_add_ms(&task->abs_deadline, task->deadline_ms);
}

void task_wait_for_period(struct task_info* task) {
  clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME,
                  &task->next_activation, NULL);
  time_add_ms(&task->next_activation, task->period_ms);
  time_add_ms(&task->abs_deadline, task->period_ms);
}

int task_create(struct task_info* task_info, void* (*func)(void*)) {
  pthread_attr_t attr;
  struct sched_param s_param;

  pthread_attr_init(&attr);
  pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
  pthread_attr_setschedpolicy(&attr, SCHED_FIFO);
  s_param.sched_priority = task_info->priority;
  pthread_attr_setschedparam(&attr, &s_param);

  int err = pthread_create(&task_info->thread_id, &attr, func, task_info);
  pthread_attr_destroy(&attr);
  return err;
}