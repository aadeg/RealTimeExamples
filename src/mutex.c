#include <pthread.h>
#include <stdio.h>

#include "rttask.h"

#define _GNU_SOURCE

struct point {
  int x;
  int y;
} point;
pthread_mutex_t point_mutex;

void* writer(void*);
void* reader(void*);

int main() {
  pthread_mutexattr_t mutext_attr;
  pthread_mutexattr_init(&mutext_attr);
  pthread_mutexattr_setprotocol(&mutext_attr, PTHREAD_PRIO_INHERIT);
  pthread_mutex_init(&point_mutex, &mutext_attr);
  pthread_mutexattr_destroy(&mutext_attr);

  pthread_t tid1, tid2;

  pthread_create(&tid1, NULL, writer, NULL);
  pthread_create(&tid2, NULL, reader, NULL);

  pthread_join(tid1, NULL);
  pthread_join(tid2, NULL);
  return 0;
}

void* writer(void* arg) {
  pthread_mutex_lock(&point_mutex);
  point.x++;
  point.y++;
  pthread_mutex_unlock(&point_mutex);
  return NULL;
}

void* reader(void* arg) {
  pthread_mutex_lock(&point_mutex);
  printf("(%d, %d)\n", point.x, point.y);
  pthread_mutex_unlock(&point_mutex);
  return NULL;
}