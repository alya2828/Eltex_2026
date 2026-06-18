#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>
#include <stdbool.h>

#define CONFIG_PATH_SIZE 256

typedef struct
{
    int client_id;
    char hostname[64];
    char name[64];
    char ip[32];
} client_config_t;

typedef struct
{
    bool cpu_enabled;
    uint32_t cpu_workers;
    uint32_t cpu_target_percent;

    bool ram_enabled;
    uint32_t ram_workers;
    uint32_t ram_size_mb;
} load_config_t;

typedef struct
{
    bool load_active;
    int last_command;
    int worker_pid;
} runtime_config_t;

typedef struct
{
    client_config_t client;
    load_config_t load;
    runtime_config_t runtime;
} config_t;

extern config_t g_config;

int config_init(void);
int config_load(const char *filename);
int config_save(const char *filename);

const char *config_get_path(void);

void config_set_defaults(void);
void config_print(void);

#endif