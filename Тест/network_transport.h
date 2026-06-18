#ifndef NETWORK_TRANSPORT_H
#define NETWORK_TRANSPORT_H

#define MAX_HOSTS 256

#define SSH_USERNAME "student"
#define SSH_PASSWORD "cluster2026"
#define REMOTE_AGENT_PATH "/tmp/agent"

int scan_local_subnet(char ***out_hosts);
void free_hosts_list(char **hosts, int count);

// Полный цикл: копирование + удалённый запуск
int ssh_deploy(const char *target_ip, const char *binary_path);

#endif