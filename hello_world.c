#include <allegro.h>
#include <stdio.h>
#include "rttask.h"

// GLOBAL CONSTANTS
#define XWIN 640
#define YWIN 480
#define XBASE 40
#define YBASE 50
#define YSPACING 30
#define BKGCOLOR 0

#define MAX_TASKS 10
#define LEN 80
#define PERIOD 30
#define PERIOD_INC 20

// GLOBAL VARIABLES
int end = 0;
char messages[MAX_TASKS][LEN + 1];
int task_ids[MAX_TASKS];
struct task_info task_infos[MAX_TASKS];

void* hello_task(void*);

void init() {
  char s[LEN];

  allegro_init();
  set_gfx_mode(GFX_AUTODETECT_WINDOWED, XWIN, YWIN, 0, 0);
  clear_to_color(screen, BKGCOLOR);
  install_keyboard();
  
  sprintf(s, "Press SPACE to create a task");
  textout_ex(screen, font, s, 10, 10, 14, BKGCOLOR);
}

void create_new_task(int i) {
  printf("Creating new task...\n");
  struct task_info* task_info = &(task_infos[i]);
  task_ids[i] = i;

  int period = PERIOD + i*PERIOD_INC;
  task_info_init(task_info, i, period, period, 50);
  task_info->arg = (void*) &task_ids[i];
  int err = task_create(task_info, hello_task);
  
  if (err)
    printf("Task %d creation error: %s", i, strerror(err));
}

int main() {
  int i = 0;
  char scan;

  init();
  do {
    scan = 0;
    if (keypressed()) {
      scan = (readkey() >> 8);
    }

    if (scan == KEY_SPACE && i < MAX_TASKS) {
      create_new_task(i);
      ++i;
    } 
  } while (scan != KEY_ESC);
  end = 1;

  for (int j = 0; j < i; ++j) {
    pthread_join(task_infos[j].thread_id, NULL);
  }

  allegro_exit();

  return 0;
}

void* hello_task(void* arg) {
  struct task_info* task_info = (struct task_info*) arg;
  char message[LEN];
  char buf[2];
  int task_id = *((int*) task_info->arg);

  sprintf(message, "I'm task %d with T = %d ms", task_id, task_info->period_ms);
  task_set_period(task_info);

  int k = 0;
  int x;
  int y = YBASE + task_id*YSPACING;
  while(!end) {
    x = XBASE + k*8;
    sprintf(buf, "%c", message[k]);
    textout_ex(screen, font, buf, x, y, 2+task_id, BKGCOLOR);

    ++k;
    if (message[k] == '\0'){
      k = 0;
      textout_ex(screen, font, message, XBASE, y, BKGCOLOR, BKGCOLOR);
    }

    if (task_deadline_missed(task_info)) {
      printf("Task %d deadline miss", task_id);
      return NULL;
    }
    task_wait_for_period(task_info);
  }
  return NULL;
}