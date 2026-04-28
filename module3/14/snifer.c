#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <signal.h>
#include <stdlib.h>

int sock;
void handler(int sig)
{
    (void)sig;//для warninga
    printf("\nStop...\n");
    close(sock);
    exit(0);
}

int main(){
    char buffer[65535];//Максимальный размер udp пакета
    signal(SIGINT, handler);
    //1 шаг подключение сокета 
    int sock=socket(AF_INET,SOCK_RAW,IPPROTO_UDP);// ipv4 raw пакеты только udp
    if (sock < 0){
        perror("socket");
        return 1;
    }
printf("Sniffer started...\n");
    //2 шаг ловим пакеты
while(1){
    int len=recvfrom(sock, buffer,sizeof(buffer), 0, NULL,  NULL);// сокет, куда положить пакет, максимум сколько читать, флаг обычный прием, от кого пакет, размер структуры адреса 
    if(len<0) //в ip|udp заголовках лежит адресотправителя
 continue;

//разбирать ip
struct iphdr *ip=(struct iphdr*)buffer;//ip указывает на ip заголовок 
//разбирать udp
struct udphdr *udp=(struct udphdr*)(buffer+ip->ihl*4);// перейти через ip заголовок


int src=ntohs(udp->source);//порт отправителя
int dst=ntohs(udp->dest);//порт получателя

// IP адреса
struct in_addr src_ip, dst_ip;
src_ip.s_addr = ip->saddr;
dst_ip.s_addr = ip->daddr;

// фильтер
if(dst == 5000 || dst == 5001)
{
    char *data = buffer + ip->ihl * 4 + 8;

    printf("Source port: %d\n", src);
    printf("Dest port: %d\n", dst);

    printf("Source IP: %s\n", inet_ntoa(src_ip));
    printf("Dest IP: %s\n", inet_ntoa(dst_ip));

    printf("Data: %s\n", data);
}
}
return 0;
}