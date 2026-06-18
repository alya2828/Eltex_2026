#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <cjson/cJSON.h>

config_t g_config;

static char g_config_path[CONFIG_PATH_SIZE];

static void config_create_dir(void)
{
    struct stat st = {0};

    if (stat("configs", &st) == -1)
    {
        mkdir("configs", 0755);
    }
}

const char *config_get_path(void)
{
    return g_config_path;
}

void config_set_defaults(void)
{
    memset(&g_config, 0, sizeof(g_config));

    gethostname(g_config.client.hostname, sizeof(g_config.client.hostname));

    if (strlen(g_config.client.hostname) == 0)
    {
        snprintf(g_config.client.hostname, sizeof(g_config.client.hostname), "unknown_host");
    }

    g_config.client.client_id = 0;

    snprintf(g_config.client.name, sizeof(g_config.client.name), "%s", g_config.client.hostname);

    snprintf(g_config.client.ip, sizeof(g_config.client.ip), "%s", "");

    g_config.load.cpu_enabled = true;
    g_config.load.cpu_workers = 1;
    g_config.load.cpu_target_percent = 80;

    g_config.load.ram_enabled = true;
    g_config.load.ram_workers = 1;
    g_config.load.ram_size_mb = 16384;

    g_config.runtime.load_active = false;
    g_config.runtime.last_command = 0;
    g_config.runtime.worker_pid = -1;
}

static char *read_file(const char *filename)
{
    FILE *file = fopen(filename, "rb");

    if (file == NULL)
    {
        return NULL;
    }

    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    rewind(file);

    char *buffer = malloc((size_t)size + 1U);

    if (buffer == NULL)
    {
        fclose(file);
        return NULL;
    }

    size_t bytes_read = fread(buffer, 1U, (size_t)size, file);

    if (bytes_read != (size_t)size)
    {
        fclose(file);
        free(buffer);
        return NULL;
    }

    buffer[size] = '\0';

    fclose(file);

    return buffer;
}

int config_init(void)
{
    char hostname[64] = {0};

    config_create_dir();

    if (gethostname(hostname, sizeof(hostname)) != 0)
    {
        snprintf(hostname, sizeof(hostname), "unknown_host");
    }

    hostname[sizeof(hostname) - 1] = '\0';

    snprintf(g_config_path, sizeof(g_config_path), "configs/%s.json", hostname);

    if (config_load(g_config_path) != 0)
    {
        printf("[CONFIG] Creating default config: %s\n", g_config_path);
        config_set_defaults();
        return config_save(g_config_path);
    }

    return 0;
}

int config_load(const char *filename)
{
    char *json_data = read_file(filename);

    if (json_data == NULL)
    {
        return -1;
    }

    cJSON *root = cJSON_Parse(json_data);

    free(json_data);

    if (root == NULL)
    {
        printf("[CONFIG] JSON parse error\n");
        return -1;
    }

    config_set_defaults();

    cJSON *client = cJSON_GetObjectItemCaseSensitive(root, "client");
    cJSON *load = cJSON_GetObjectItemCaseSensitive(root, "load");
    cJSON *runtime = cJSON_GetObjectItemCaseSensitive(root, "runtime");

    if (client != NULL)
    {
        cJSON *client_id = cJSON_GetObjectItemCaseSensitive(client, "client_id");
        cJSON *hostname = cJSON_GetObjectItemCaseSensitive(client, "hostname");
        cJSON *name = cJSON_GetObjectItemCaseSensitive(client, "name");
        cJSON *ip = cJSON_GetObjectItemCaseSensitive(client, "ip");

        if (cJSON_IsNumber(client_id))
            g_config.client.client_id = client_id->valueint;

        if (cJSON_IsString(hostname) && hostname->valuestring != NULL)
            snprintf(g_config.client.hostname, sizeof(g_config.client.hostname), "%s", hostname->valuestring);

        if (cJSON_IsString(name) && name->valuestring != NULL)
            snprintf(g_config.client.name, sizeof(g_config.client.name), "%s", name->valuestring);

        if (cJSON_IsString(ip) && ip->valuestring != NULL)
            snprintf(g_config.client.ip, sizeof(g_config.client.ip), "%s", ip->valuestring);
    }

    if (load != NULL)
    {
        cJSON *cpu_enabled = cJSON_GetObjectItemCaseSensitive(load, "cpu_enabled");
        cJSON *cpu_workers = cJSON_GetObjectItemCaseSensitive(load, "cpu_workers");
        cJSON *cpu_target_percent = cJSON_GetObjectItemCaseSensitive(load, "cpu_target_percent");

        cJSON *ram_enabled = cJSON_GetObjectItemCaseSensitive(load, "ram_enabled");
        cJSON *ram_workers = cJSON_GetObjectItemCaseSensitive(load, "ram_workers");
        cJSON *ram_size_mb = cJSON_GetObjectItemCaseSensitive(load, "ram_size_mb");

        if (cJSON_IsBool(cpu_enabled))
            g_config.load.cpu_enabled = cJSON_IsTrue(cpu_enabled);

        if (cJSON_IsNumber(cpu_workers))
            g_config.load.cpu_workers = (uint32_t)cpu_workers->valueint;

        if (cJSON_IsNumber(cpu_target_percent))
            g_config.load.cpu_target_percent = (uint32_t)cpu_target_percent->valueint;

        if (cJSON_IsBool(ram_enabled))
            g_config.load.ram_enabled = cJSON_IsTrue(ram_enabled);

        if (cJSON_IsNumber(ram_workers))
            g_config.load.ram_workers = (uint32_t)ram_workers->valueint;

        if (cJSON_IsNumber(ram_size_mb))
            g_config.load.ram_size_mb = (uint32_t)ram_size_mb->valueint;
    }

    if (runtime != NULL)
    {
        cJSON *load_active = cJSON_GetObjectItemCaseSensitive(runtime, "load_active");
        cJSON *last_command = cJSON_GetObjectItemCaseSensitive(runtime, "last_command");
        cJSON *worker_pid = cJSON_GetObjectItemCaseSensitive(runtime, "worker_pid");

        if (cJSON_IsBool(load_active))
            g_config.runtime.load_active = cJSON_IsTrue(load_active);

        if (cJSON_IsNumber(last_command))
            g_config.runtime.last_command = last_command->valueint;

        if (cJSON_IsNumber(worker_pid))
            g_config.runtime.worker_pid = worker_pid->valueint;
    }

    cJSON_Delete(root);

    printf("[CONFIG] Loaded: %s\n", filename);

    return 0;
}

int config_save(const char *filename)
{
    cJSON *root = cJSON_CreateObject();

    if (root == NULL)
        return -1;

    cJSON *client = cJSON_AddObjectToObject(root, "client");
    cJSON *load = cJSON_AddObjectToObject(root, "load");
    cJSON *runtime = cJSON_AddObjectToObject(root, "runtime");

    cJSON_AddNumberToObject(client, "client_id", g_config.client.client_id);
    cJSON_AddStringToObject(client, "hostname", g_config.client.hostname);
    cJSON_AddStringToObject(client, "name", g_config.client.name);
    cJSON_AddStringToObject(client, "ip", g_config.client.ip);

    cJSON_AddBoolToObject(load, "cpu_enabled", g_config.load.cpu_enabled);
    cJSON_AddNumberToObject(load, "cpu_workers", g_config.load.cpu_workers);
    cJSON_AddNumberToObject(load, "cpu_target_percent", g_config.load.cpu_target_percent);

    cJSON_AddBoolToObject(load, "ram_enabled", g_config.load.ram_enabled);
    cJSON_AddNumberToObject(load, "ram_workers", g_config.load.ram_workers);
    cJSON_AddNumberToObject(load, "ram_size_mb", g_config.load.ram_size_mb);

    cJSON_AddBoolToObject(runtime, "load_active", g_config.runtime.load_active);
    cJSON_AddNumberToObject(runtime, "last_command", g_config.runtime.last_command);
    cJSON_AddNumberToObject(runtime, "worker_pid", g_config.runtime.worker_pid);

    char *json_string = cJSON_Print(root);

    if (json_string == NULL)
    {
        cJSON_Delete(root);
        return -1;
    }

    FILE *file = fopen(filename, "w");

    if (file == NULL)
    {
        perror("[CONFIG] fopen");
        cJSON_free(json_string);
        cJSON_Delete(root);
        return -1;
    }

    fprintf(file, "%s\n", json_string);

    fclose(file);

    cJSON_free(json_string);
    cJSON_Delete(root);

    printf("[CONFIG] Saved: %s\n", filename);

    return 0;
}

void config_print(void)
{
    printf("\n CONFIG \n");

    printf("Config path        : %s\n", g_config_path);
    printf("Client ID          : %d\n", g_config.client.client_id);
    printf("Hostname           : %s\n", g_config.client.hostname);
    printf("Client name        : %s\n", g_config.client.name);
    printf("Client IP          : %s\n", g_config.client.ip);

    printf("CPU enabled        : %d\n", g_config.load.cpu_enabled);
    printf("CPU workers        : %u\n", g_config.load.cpu_workers);
    printf("CPU target         : %u%%\n", g_config.load.cpu_target_percent);

    printf("RAM enabled        : %d\n", g_config.load.ram_enabled);
    printf("RAM workers        : %u\n", g_config.load.ram_workers);
    printf("RAM size           : %u MB\n", g_config.load.ram_size_mb);

    printf("Load active        : %d\n", g_config.runtime.load_active);
    printf("Last command       : %d\n", g_config.runtime.last_command);
    printf("Worker PID         : %d\n", g_config.runtime.worker_pid);

}