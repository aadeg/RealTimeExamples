#include <pthread.h>
#include <stdio.h>

void* task(void* arg);

int main() {
  pthread_attr_t task_attr;
  struct sched_param param;

  pthread_attr_init(&task_attr);
  pthread_attr_setinheritsched(&task_attr, PTHREAD_EXPLICIT_SCHED);
  pthread_attr_setschedpolicy(&task_attr, SCHED_FIFO);
  param.sched_priority = 23;
  pthread_attr_setschedparam(&task_attr, &param);



  pthread_t task_id_1, task_id_2;
  int task_ret_1, task_ret_2;
  int a = 1;
  int b = 2;

  task_ret_1 = pthread_create(&task_id_1, &task_attr, task, (void*) &a);
  task_ret_2 = pthread_create(&task_id_2, &task_attr, task, (void*) &b);

  pthread_join(task_id_1, NULL);
  pthread_join(task_id_2, NULL);
  

  printf("Thread 1 returns %d\n", task_ret_1);
  printf("Thread 2 returns %d\n", task_ret_2);

  pthread_attr_destroy(&task_attr);
  return 0;
}

void* task(void* p) {
  int* pi = (int*) p;
  
  printf("This is TASK %d\n", *pi);
  return NULL;
}