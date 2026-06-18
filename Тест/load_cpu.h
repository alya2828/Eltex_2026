#ifndef LOAD_CPU_H
#define LOAD_CPU_H

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <pthread.h>
#include <sys/sysinfo.h>
#include <stdint.h>

#define WORK_US_DEFAULT 100000
#define BLOCK_ITERATIONS_DEFAULT 150000

int cpu_load_init(int load_percent);

#endif