#ifndef LOAD_RAM_H
#define LOAD_RAM_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/mman.h>
#include <unistd.h>
#include <stdint.h>

#define CPU_IDLE_USLEEP 1000
#define XORSHIFT_INIT_STATE 2463534242u
#define ITERATIONS_PER_SLEEP 10000

int ram_load_init(size_t desired_size);

#endif