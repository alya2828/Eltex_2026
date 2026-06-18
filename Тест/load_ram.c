#include "load_ram.h"

static void *l_memory = NULL;
static size_t l_size = 0;
static volatile sig_atomic_t l_running = 0;

static int ram_allocate(size_t size);
static void ram_touch_init(void);
static void ram_touch(void);
static void ram_release(void);

static void stop_handler(int sig) {
    (void)sig;
    l_running = 0;
}

int ram_load_init(size_t desired_size) {
    // Настройка обработчика сигналов
    struct sigaction sa;
    sa.sa_handler = stop_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGTERM, &sa, NULL);

    // Отключаем OOM-killer
    FILE *f = fopen("/proc/self/oom_score_adj", "w");
    if (f) {
        fprintf(f, "-1000");
        fclose(f);
    }

    if (ram_allocate(desired_size) != 0) {
        return -1;
    }

    if (mlock(l_memory, l_size) != 0) {
        perror("mlock");
    }

    l_running = 1;
    ram_touch_init();
    ram_touch();
    ram_release();
    return 0;
}

static int ram_allocate(size_t size) {
    if (size == 0) {
        return -1;
    }

    if (l_memory != NULL) {
        return -1;
    }

    void *ptr = NULL;
    size_t alloc_size = size;
    while (alloc_size > 0) {
        ptr = malloc(alloc_size);
        if (ptr != NULL) {
            break;
        }
        alloc_size /= 2; // уменьшаем размер, если выделить не удалось
    }

    if (ptr == NULL) {
        return -1; // не удалось выделить память
    }

    l_memory = ptr;
    l_size = alloc_size;
    return 0;
}

static void ram_touch_init(void) {
    volatile unsigned char *p = (volatile unsigned char *)l_memory;
    for (size_t i = 0; i < l_size; i++) {
        p[i] = (unsigned char)(i);
    }
}

static void ram_touch(void) {
    uint32_t state = XORSHIFT_INIT_STATE;
    volatile unsigned char *p = (volatile unsigned char *)l_memory;

    while (l_running) {
        for (unsigned int cnt = 0; cnt < ITERATIONS_PER_SLEEP; cnt++) {
            // Генерация псевдослучайного 32-битного числа
            state ^= state << 13;
            state ^= state >> 17;
            state ^= state << 5;
            
            // Преобразуем псевдослучайное число в индекс в пределах [0, l_size-1]
            size_t idx = state % l_size;
            p[idx] = (unsigned char)(idx);
        }
        usleep(CPU_IDLE_USLEEP);  // снижает загрузку CPU за счёт менее частых обращений к памяти
    }
}

static void ram_release(void) {
    l_running = 0;
    if (l_memory) {
        free(l_memory);
        l_memory = NULL;
    }
    l_size = 0;
}