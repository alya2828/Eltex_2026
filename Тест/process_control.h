#ifndef PROCESS_CONTROL_H
#define PROCESS_CONTROL_H

#include <unistd.h>
#include <sys/types.h>

typedef struct
{
    pid_t pid;                 
    pid_t ppid;               
    char process_name[64];   
    int active;              
} process_info_t;

int process_init(void);

pid_t process_create(void);

// Установка имени процесса для логов и хранения информации о процессе
int process_set_name(const char *name);

pid_t process_get_pid(void);

pid_t process_get_parent_pid(void);

int process_is_alive(pid_t pid);

int process_terminate(pid_t pid);

int hide_process(void);


#endif