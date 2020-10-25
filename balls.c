#include <stdio.h>
#include <allegro.h>
#include <stdbool.h>
#include <math.h>

#include "rttask.h"

// GLOBAL CONSTANTS
#define WIN_WIDTH    640
#define WIN_HEIGHT   480
#define BKG_COLOR    0

#define BOX_WIDTH    489
#define BOX_HEIGHT   399
#define BOX_X        5
#define BOX_Y        75
#define FLOOR_Y      5       


// TASK CONSTANTS
#define PERIOD      20
#define DEADLINE    20
#define PRIORITY    80
#define MAX_TASKS   MAX_BALLS + 2

// BALL CONSTANTS
#define MAX_BALLS   20
#define G0          9.8
#define MIN_RADIUS  5
#define MAX_RADIUS  10
#define MIN_HEIGHT  200
#define MAX_HEIGHT  390
#define MIN_VX      10
#define MAX_VX      20
#define FRICTION    0.95
#define DUMP        0.9
#define TSCALE      10

#define _GNU_SOURCE

typedef struct {
  int color;
  float radius;   // (m)
  float x;        // (m)
  float y;        // (m)
  float vx;       // (m/s)
  float vy;       // (m/s)
  float v0;       // (m/s)
} ball_t;

typedef struct {
  ball_t ball;
  pthread_mutex_t mutex;
} mutex_ball_t;

int n_balls = 0;
bool end = false;
float g = G0;

mutex_ball_t balls[MAX_BALLS];
struct task_info task_infos[MAX_TASKS];

char get_scancode() {
  if (keypressed()) {
    return readkey() >> 8;
  }
  return 0;
}

float rand_float(float min, float max) {
  float r = rand() / (float) RAND_MAX;
  return min + (max - min) * r;
}

void ball_draw(const ball_t* ball) {
  int x = BOX_X + 1 + ball->x;
  int y = WIN_HEIGHT - FLOOR_Y - ball->y;
  circlefill(screen, x, y, ball->radius, ball->color);
}

void ball_handle_bounce(ball_t* ball) {
  if (ball->y <= ball->radius) {
    ball->y = ball->radius;
    ball->vy = -DUMP * ball->vy;
  }
  if (ball->y >= BOX_HEIGHT - ball->radius) {
    ball->y = BOX_HEIGHT - ball->radius;
    ball->vy *= -DUMP;
  }
  if (ball->x >= BOX_WIDTH - ball->radius) {
    ball->x = BOX_WIDTH - ball->radius;
    ball->vx *= -1;
  }
  if (ball->x <= ball->radius) {
    ball->x = ball->radius;
    ball->vx *= -1;
  }
}

void ball_handle_friction(ball_t* ball) {
  if (ball->y <= ball->radius && abs(ball->vy) <= 0.01) {
    ball->vx *= FRICTION;
  }
}

void ball_init(ball_t* ball, int color) {
  ball->color = color;
  ball->radius = rand_float(MIN_RADIUS, MAX_RADIUS);
  ball->x = ball->radius + 1;
  ball->y = rand_float(MIN_HEIGHT, MAX_HEIGHT);
  ball->vx = rand_float(MIN_VX, MAX_VX);
  ball->vy = 0;
  ball->v0 = sqrt(2*g*ball->y);
}

void* ball_task(void* arg) {
  struct task_info* task_info = (struct task_info*) arg;
  mutex_ball_t* ball = (mutex_ball_t*) task_info->arg;

  pthread_mutex_lock(&(ball->mutex));
  ball_init(&(ball->ball), 2 + task_info->task_num);
  pthread_mutex_unlock(&(ball->mutex));
  float dt = TSCALE * ((float) task_info->period_ms) / 1000.0;

  task_set_period(task_info);

  ball_t local_ball;
  while (!end) {
    pthread_mutex_lock(&(ball->mutex));
    local_ball = ball->ball;
    pthread_mutex_unlock(&(ball->mutex));

    local_ball.vy -= g*dt;
    local_ball.x += local_ball.vx*dt;
    local_ball.y += local_ball.vy*dt - g*dt*dt/2;
    ball_handle_bounce(&local_ball);
    ball_handle_friction(&local_ball);

    pthread_mutex_lock(&(ball->mutex));
    ball->ball = local_ball;
    pthread_mutex_unlock(&(ball->mutex));

    if (task_deadline_missed(task_info)) {
      printf("Box task - deadline miss!\n");
    }
    task_wait_for_period(task_info);
  }

  return NULL;
}

void create_new_ball_task(int i) {
  struct task_info* task_info = &task_infos[i];
  task_info_init(task_info, i, PERIOD, DEADLINE, PRIORITY);
  task_info->arg = &(balls[i]);
  int err = task_create(task_info, ball_task);
  if (err)
    printf("Error - ball task %d create - %s", i, strerror(err));
}

void* display_task(void* arg) {
  struct task_info* task_info = (struct task_info*) arg;

  task_set_period(task_info);
  ball_t local_ball;
  while (!end) {
    rectfill(screen, BOX_X + 1, BOX_Y + 1,
             BOX_X + BOX_WIDTH, BOX_Y + BOX_HEIGHT, BKG_COLOR);
    
    for (int i = 0; i < n_balls; ++i) {
      pthread_mutex_lock(&(balls[i].mutex));
      local_ball = balls[i].ball;
      pthread_mutex_unlock(&(balls[i].mutex));
      ball_draw(&local_ball);
    }

    if (task_deadline_missed(task_info))
      printf("Display task - deadline miss!\n");
    task_wait_for_period(task_info);
  }

  return NULL;
}

void* interp_task(void* arg) {
  struct task_info* task_info = (struct task_info*) arg;

  task_set_period(task_info);
  char scan;
  do {
    scan = get_scancode();
    switch (scan) {
      case KEY_SPACE:
        if (n_balls < MAX_BALLS) {
          create_new_ball_task(n_balls);
          ++n_balls;
        }
        break;
      case KEY_UP:
        g = g + 1;
        break;
      case KEY_DOWN:
        if (g > 1) g = g - 1;
        break;
      default: break;
    }

    if (task_deadline_missed(task_info)) {
      printf("Interp task - deadline miss!\n");
    }
    task_wait_for_period(task_info);
  } while (scan != KEY_ESC);

  end = true;
  return NULL;
}

void init_mutex() {
  pthread_mutexattr_t attr;
  pthread_mutexattr_init(&attr);
  pthread_mutexattr_setprotocol(&attr, PTHREAD_PRIO_INHERIT);
  
  for (int i = 0; i < MAX_BALLS; ++i) {  
    pthread_mutex_init(&(balls[i].mutex), &attr);
  }

  pthread_mutexattr_destroy(&attr);
}

void init() {
  allegro_init();
  set_gfx_mode(GFX_AUTODETECT_WINDOWED, WIN_WIDTH, WIN_HEIGHT, 0, 0);
  clear_to_color(screen, BKG_COLOR);
  install_keyboard();
  srand(time(NULL));

  init_mutex();

  struct task_info* display_task_info = &task_infos[MAX_TASKS - 2];
  task_info_init(display_task_info, MAX_TASKS - 2, PERIOD, DEADLINE, PRIORITY);
  int err = task_create(display_task_info, display_task);
  if (err)
    printf("Error - display task create - %s", strerror(err));

  struct task_info* interp_task_info = &task_infos[MAX_TASKS - 1];
  task_info_init(interp_task_info, MAX_TASKS - 1, PERIOD, DEADLINE, PRIORITY);
  err = task_create(interp_task_info, interp_task);
  if (err)
    printf("Error - interp task create - %s", strerror(err));
}

int main() {
  init();

  for (int i = 0; i < MAX_TASKS; ++i) {
    pthread_join(task_infos[i].thread_id, NULL);
  }
  allegro_exit();
  return 0;

  return 0;
}