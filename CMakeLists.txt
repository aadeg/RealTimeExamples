cmake_minimum_required(VERSION 2.8.3)
project(mpcplanner)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-Wpedantic -Wall ${CMAKE_CXX_FLAGS}")

link_directories(/usr/lib/x86_64-linux-gnu)
include_directories(
  include/
)

add_executable(create
  src/create.c
)
target_link_libraries(create
  pthread
  rt
)

add_executable(periodic_task
  src/periodic_task.c
  src/rttask.c
)
target_link_libraries(periodic_task
  pthread
  rt
)

add_executable(mutex
  src/mutex.c
  src/rttask.c
)
target_link_libraries(mutex
  pthread
  rt
)

add_executable(hello_world
  src/hello_world.c
  src/rttask.c
)
target_link_libraries(hello_world
  pthread
  rt
  alleg
)

add_executable(balls
  src/balls.c
  src/rttask.c
)
target_link_libraries(balls
  pthread
  rt
  alleg
  m
)