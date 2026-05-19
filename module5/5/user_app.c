// Source - https://stackoverflow.com/a/27782733
// Posted by K_K, modified by community. See post 'Timeline' for change history
// Retrieved 2026-05-20, License - CC BY-SA 3.0

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <unistd.h>

#define NETLINK_USER 31 //Номер пользовательского Netlink protocol c ядром должны быть одинковые
#define MAX_MSG_SIZE 10 // размер сообщения

#define MAX_PAYLOAD 1024 /* maximum payload size*/

struct sockaddr_nl src_addr, dest_addr;// адрес текущего процесса (пользователь, ядро)
struct nlmsghdr *nlh = NULL;// Указатель на netlink message header
struct iovec iov;//структура для передачи данных хранит(адрес данных, размер данных)
int sock_fd;
struct msghdr msg;

int main()
{
	 char input[MAX_MSG_SIZE];
	 
    sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
    if (sock_fd < 0)
        return -1;

    memset(&src_addr, 0, sizeof(src_addr));//заполняем структуру нулями 
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid(); // получаем pid процесса

    bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr));// bind регестирует socket в системе

    memset(&dest_addr, 0, sizeof(dest_addr));//очищаем структуру ядра
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0; /* For Linux Kernel */
    dest_addr.nl_groups = 0; // unicast сообщение только одному получателю

    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));//расчитываем размер сообщения для netlink, NLMSG_SPACE рассчитывает размер сообщенияс учетом заголовка
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));//очищаем выделенную память
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);//размер netlink message
    nlh->nlmsg_pid = getpid();//Kernel узнает кто отправил сообщение
    nlh->nlmsg_flags = 0;
    
    printf("Enter message (max 9 chars): ");
	fgets(input, MAX_MSG_SIZE, stdin);
	input[strcspn(input, "\n")] = '\0';// заменим конец строки \n на \0
	strcpy( NLMSG_DATA(nlh),input);//  Копируем сообщение пользователя в payload netlink сообщения

	// Настройка структуры iovec
	
    iov.iov_base = (void *)nlh;// адрес данных
    iov.iov_len = nlh->nlmsg_len;// размер
    
    // Настройка структуры сообщения
    
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1; // количество элемнтов iovec(один буфер)

    printf("Sending message to kernel\n");
    sendmsg(sock_fd, &msg, 0);// sendmsg отправляет netlink message
    printf("Waiting for message from kernel\n");

    /* Read message from kernel */
    if (recvmsg(sock_fd, &msg, 0) < 0)
		{
    perror("recvmsg");
    return -1;
		}
    printf("Received message payload: %s\n", (char *)NLMSG_DATA(nlh));	// добавим тут явное приведение типа для коммпилятора
    free(nlh);
    close(sock_fd);
    
    return 0;
}
