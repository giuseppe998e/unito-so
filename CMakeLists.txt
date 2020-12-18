cmake_minimum_required(VERSION 3.13)
project(unito_so C)

set(CMAKE_C_STANDARD 11)

add_executable(unito_so src/main.c src/global.h src/config/config.c src/config/config.h src/student/student.c src/student/student.h src/utils/sem.c src/utils/sem.h src/utils/shm.c src/utils/shm.h src/utils/msg.c src/utils/msg.h)
