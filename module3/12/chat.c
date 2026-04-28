#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <signal.h>

#define BUF_SIZE 1024

int sock;

void handler(int sig)
{
    close(sock);
    exit(0);
}

int main(int argc,char *argv[])
{
    if(argc!=3)
    {
        printf("Usage: %s my_port peer_port\n",argv[0]);
        return 1;
    }

    int my_port=atoi(argv[1]);
    int peer_port=atoi(argv[2]);

    signal(SIGINT,handler);

    sock=socket(AF_INET,SOCK_DGRAM,0); // ipv4 udp протокол автоматически

    if(sock<0)
    {
        perror("socket");
        return 1;
    }

    struct sockaddr_in my_addr;
    memset(&my_addr,0,sizeof(my_addr)); // адрес клиента, обнуляем всю структуру

    my_addr.sin_family=AF_INET;// ipv4
    my_addr.sin_port=htons(my_port);
    my_addr.sin_addr.s_addr=INADDR_ANY;// слушать на всех интерфейсах 

    if(bind(sock,(struct sockaddr*)&my_addr,sizeof(my_addr))<0)
    {
        perror("bind");
        return 1;//?
    }

    struct sockaddr_in peer_addr;//адрес клиента
    memset(&peer_addr,0,sizeof(peer_addr));

    peer_addr.sin_family=AF_INET;
    peer_addr.sin_port=htons(peer_port);
    inet_pton(AF_INET,"127.0.0.1",&peer_addr.sin_addr);// преобразование в бинарный ip

    pid_t pid=fork();

    if(pid==0)
    {
        // дочерний процесс принимает сообщения
        char buffer[BUF_SIZE];

        while(1)
        {

            int n=recvfrom( // получение пакета
                sock,
                buffer,
                BUF_SIZE-1,
                0,
                NULL,
                NULL
            );

            if(n>0)
            {
                buffer[n]='\0';
                printf("\nFriend: %s",buffer);
                fflush(stdout);
            }
        }
    }
    else
    {
        // родитель отправляет
        char buffer[BUF_SIZE];

        while(1)
        {
            fgets(buffer,BUF_SIZE,stdin);

            sendto( //отправка udp пакета
                sock,
                buffer,
                strlen(buffer),
                0,
                (struct sockaddr*)&peer_addr,
                sizeof(peer_addr)
            );
        }
    }

    return 0;
}