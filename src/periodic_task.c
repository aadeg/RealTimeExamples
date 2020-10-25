#include <stdio.h>
#include <pthread.h>
#include <time.h>
#include <string.h>

#include "rttask.h"

void* task(void* arg);

int main() {
  struct task_info task_info;
  task_info_init(&task_info, 1, 100, 80, 20);

  int err;

  err = task_create(&task_info, task);
  if (err) {
    printf("%s\n", strerror(err));
  }

  printf("Here\n");
  pthread_join(task_info.thread_id, NULL);
  return 0;
}

void* task(void* arg) {
  struct task_info* task_info = (struct task_info*) arg;
  task_set_period(task_info);
  printf("here task\n");
  while(1) {
    printf(".\n");

    if (task_deadline_missed(task_info))
      printf("\n-!-\n");
    task_wait_for_period(task_info);
  }

  return NULL;
}