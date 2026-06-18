#include "load_cpu.h"

static volatile sig_atomic_t l_running = 0;
static uint64_t block_iterations = BLOCK_ITERATIONS_DEFAULT;
static int target_percent = 100;
static uint64_t work_us = WORK_US_DEFAULT;
static uint64_t sleep_us = 0;

static int num_threads = 0;
static pthread_t *threads = NULL;
static uint8_t *threads_errors = NULL;
static pthread_mutex_t stop_mutex = PTHREAD_MUTEX_INITIALIZER;

int cpu_load_init(int load_percent);
static int cpu_load_start(void);
static void cpu_load_stop(void);
static uint64_t get_usec(void);
static void *cpu_worker(void *arg);
static void cpu_math_operation(uint64_t count);

static void stop_handler(int sig)
{
    (void)sig;
    l_running = 0;
}

int cpu_load_init(int load_percent)
{
    if (load_percent <= 0 || load_percent > 100)
    {
        return -1;
    }

    target_percent = load_percent;

    sleep_us = work_us * (100 - target_percent) / target_percent;

    num_threads = sysconf(_SC_NPROCESSORS_ONLN);
    if (num_threads < 1)
    {
        num_threads = 1;
    }

    threads = (pthread_t *)calloc(num_threads, sizeof(pthread_t));
    if (!threads)
    {
        return -1;
    }
    threads_errors = (uint8_t *)calloc(num_threads, sizeof(uint8_t));
    if (!threads_errors){
        free(threads);
        return -1;
    }

    struct sigaction sa;
    sa.sa_handler = stop_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGTERM, &sa, NULL);

    l_running = 1;
    cpu_load_start();
    return 0;
}

static int cpu_load_start(void)
{
    if (!l_running || threads == NULL)
    {
        return -1;
    }

    for (int i = 0; i < num_threads; i++)
    {
        if (pthread_create(&threads[i], NULL, cpu_worker, NULL) != 0)
        {
            threads_errors[i] = 1;
        }
    }

    while (l_running)
    {
        pause();
    }
    cpu_load_stop();

    return 0;
}

static void cpu_load_stop(void)
{
    if (threads)
    {
        l_running = 0;
        for (int i = 0; i < num_threads; i++)
        {
            if (!threads_errors[i])
            {
                pthread_join(threads[i], NULL);
            }
        }
        free(threads);
        free(threads_errors);
        threads = NULL;
    }
    num_threads = 0;
}

static uint64_t get_usec(void) // Получение текущего времени в микросекундах
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000L + ts.tv_nsec / 1000L;
}

static void *cpu_worker(void *arg)
{
    (void)arg;

    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGTERM);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);

    uint64_t start_time, elapsed;

    while (l_running)
    {
        start_time = get_usec();

        cpu_math_operation(block_iterations);

        elapsed = get_usec() - start_time;

        while (elapsed < work_us && l_running) // Если отработали меньше work_us, добавляем ещё итераций
        {
            cpu_math_operation(block_iterations);

            elapsed = get_usec() - start_time;
        }

        if (sleep_us > 0 && l_running)
        {
            usleep(sleep_us);
        }
    }
    return NULL;
}

static void cpu_math_operation(uint64_t count) {
    volatile double sum = 0.0;
    volatile float fsum = 0.0f;
    volatile unsigned long long intsum = 0;
    volatile float f_acc = 0.0f;
    volatile unsigned long long int_acc = 0;
    for (uint64_t i = 0; i < count; i++) {
        double x = (double)i * 0.0001;
        double y = fabs(x) + 0.5;
        double a = sin(x) * cos(x) + sqrt(y);
        double b = log(y + 1.0) * exp(-x / 100.0);
        double c = pow(a, 1.5) + tgamma(b + 1.0) / (a + 1.0);
        double d = erf(c) * hypot(a, b) + lgamma(fabs(c) + 1.0);
        sum += d;
        fsum += (float)sum;
        intsum += (unsigned long long)(sum * 1e9) ^ 0x9e3779b97f4a7c15ULL;
        if (i % 10 == 0) {
            sum = sin(sum) * cos(sum) + sqrt(fabs(sum) + 1.0);
        }
        double sin_x = sin(x);
        double cos_x = cos(x);
        double sqrt_x = sqrt(fabs(x) + 0.5);
        double log_x = log(fabs(x) + 1.0);
        double exp_x = exp(-x / 1000.0);
        sum += sin_x * cos_x + sqrt_x * log_x + exp_x;
        float fx = (float)x;
        f_acc += sinf(fx) * cosf(fx) + sqrtf(fabsf(fx) + 0.5f);
        unsigned long long bit = (unsigned long long)(x * 1000000.0);
        int_acc ^= bit;
        int_acc = (int_acc << 7) | (int_acc >> 57);
        int_acc *= 0x9e3779b97f4a7c15ULL;
    }
}